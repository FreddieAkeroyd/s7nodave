#include <cmath>

#include <stdio.h>

#include <boost/make_shared.hpp>

#include <epicsGuard.h>

#include "S7nodavePollGroup.h"

using boost::make_shared;
using boost::optional;
using boost::shared_ptr;
using boost::tuple;
using std::list;
using std::map;
using std::pair;
using std::string;
using std::set;

typedef tuple<S7nodavePlcAddress, int, void *> PollRequest;
typedef list<PollRequest> PollRequestList;
typedef tuple<S7nodavePlcAddress, int, void *, S7nodavePollRequester *> PollRequestWithRequester;
typedef list<PollRequestWithRequester> PollRequestWithRequesterList;
typedef set<S7nodavePollRequester *> PollRequesterSet;
typedef map< pair<string, string>, shared_ptr<S7nodavePollGroup> > PollGroupMap;
typedef pair< pair<string, string>, shared_ptr<S7nodavePollGroup> > PollGroupEntry;

void S7nodavePollService::requestRead(S7nodavePlcAddress plcAddress, int bufferSize, void *buffer)
{
    PollRequest request(plcAddress, bufferSize, buffer);
    this->requests.push_back(request);
}

void S7nodavePollGroup::registerRequester(S7nodavePollRequester *requester)
{
    epicsGuard<epicsMutex> guard(this->instanceMutex);
    this->requesters.insert(requester);
}

void S7nodavePollGroup::unregisterRequester(S7nodavePollRequester *requester)
{
    epicsGuard<epicsMutex> guard(this->instanceMutex);
    PollRequesterSet::iterator i = this->requesters.find(requester);
    if (i != this->requesters.end()) {
        this->requesters.erase(i);
    }
}

void S7nodavePollGroup::create(string portName, string pollGroupName, double pollingInterval, unsigned int priority)
{
    epicsGuard<epicsMutex> guard(staticMutex);
    if (pollGroups.find(pair<string, string>(portName, pollGroupName)) != pollGroups.end()) {
        printf("Error: Duplicate definition of poll group \"%s\" for port \"%s\".\n", pollGroupName.c_str(), portName.c_str());
        return;
    }
    shared_ptr<S7nodavePollGroup> pollGroup = shared_ptr<S7nodavePollGroup>(new S7nodavePollGroup(portName, pollGroupName, pollingInterval, priority));
    if (pollGroup->initAsyn() != asynSuccess) {
        return;
    }
    pollGroups.insert(PollGroupEntry(pair<string, string>(portName, pollGroupName), pollGroup));
}

optional<S7nodavePollGroup&> S7nodavePollGroup::find(string portName, string pollGroupName)
{
    epicsGuard<epicsMutex> guard(staticMutex);
    PollGroupMap::iterator i = pollGroups.find(pair<string, string>(portName, pollGroupName));
    if (i == pollGroups.end()) {
        return optional<S7nodavePollGroup&>();
    } else {
        return *i->second;
    }
}

void S7nodavePollGroup::startPollGroups()
{
    epicsGuard<epicsMutex> guard(staticMutex);
    if (isStarted) {
        return;
    }
    for (PollGroupMap::iterator i = pollGroups.begin(); i != pollGroups.end(); i++) {
        S7nodavePollGroup& pollGroup = *i->second;
        pollGroup.timer.start(pollGroup.timerNotify, pollGroup.pollingInterval);
    }
    isStarted = true;
}

epicsTimerNotify::expireStatus S7nodavePollGroup::TimerNotify::expire(const epicsTime& currentTime)
{
    // Save start time
    epicsTime startTime = epicsTime::getCurrent();

    // Do polling
    this->pollGroup->process();

    // Schedule next run
    epicsTime endTime = epicsTime::getCurrent();
    double timeConsumed = endTime - startTime;
    double delay = this->pollGroup->pollingInterval;
    double timeToWait = delay - timeConsumed;
    if (timeToWait <= 0) {
        timeToWait = std::min(delay, 0.1);
    }
    return expireStatus(restart, timeToWait);
}

S7nodavePollGroup::S7nodavePollGroup(std::string portName, std::string pollGroupName, double pollingInterval, unsigned int priority) :
                name(pollGroupName),
                portName(portName),
                pollingInterval(pollingInterval),
                priority(priority),
                timerQueue(epicsTimerQueueActive::allocate(false, priority)),
                timer(timerQueue.createTimer()),
                timerNotify(TimerNotify(this)),
                myAsynUser(NULL),
                myAsynS7nodaveInterface(NULL),
                myAsynS7nodave(NULL)
{
}

S7nodavePollGroup::~S7nodavePollGroup()
{
    this->timer.destroy();
    this->timerQueue.release();
    if (this->myAsynUser) {
        pasynManager->disconnect(this->myAsynUser);
        pasynManager->freeAsynUser(this->myAsynUser);
        this->myAsynUser = NULL;
        this->myAsynS7nodaveInterface = NULL;
        this->myAsynS7nodave = NULL;
    }
}

asynStatus S7nodavePollGroup::initAsyn()
{
    // createAsynUser always succeeds (otherwise the thread is suspended)
    // We do not pass a call-back function, because we only use asyn
    // synchronously.
    this->myAsynUser = pasynManager->createAsynUser(NULL, NULL);
    this->myAsynUser->userPvt = this;

    // Connect asynUser to device
    const int deviceAddress = 0;
    asynStatus status = pasynManager->connectDevice(this->myAsynUser, this->portName.c_str(), deviceAddress);
    if (status != asynSuccess) {
        asynPrint(this->myAsynUser, ASYN_TRACE_ERROR, "port \"%s\" poll group \"%s\" S7nodavePollGroup::initAsyn pasynManager->connectDevice %s\n", this->portName.c_str(), this->name.c_str(), this->myAsynUser->errorMessage);
        return asynError;
    }

    // Get device specific interface
    this->myAsynS7nodaveInterface = pasynManager->findInterface(this->myAsynUser, asynS7nodaveType, 1);
    if (this->myAsynS7nodaveInterface == NULL) {
        asynPrint(this->myAsynUser, ASYN_TRACE_ERROR, "port \"%s\" poll group \"%s\" S7nodavePollGroup::initAsyn pasynManager->findInterfacce %s\n", this->portName.c_str(), this->name.c_str(), this->myAsynUser->errorMessage);
        pasynManager->disconnect(this->myAsynUser);
        return asynError;
    }
    this->myAsynS7nodave = static_cast<asynS7nodave *>(this->myAsynS7nodaveInterface->pinterface);

    return asynSuccess;
}

void S7nodavePollGroup::process()
{
    PollRequesterSet requesters;
    {
        // Lock mutex while making a copy of the list of requesters.
        epicsGuard<epicsMutex> guard(this->instanceMutex);
        requesters = this->requesters;
    }
    if (requesters.size() == 0) {
        return;
    }

    PollRequestWithRequesterList requests;
    S7nodavePollService service;
    for (PollRequesterSet::iterator i = requesters.begin(); i != requesters.end(); i++) {
        S7nodavePollRequester *requester = *i;
        // Ask requester to queue read requests.
        requester->prepareRequest(service);
        // Transfer
        for (PollRequestList::iterator i2 = service.requests.begin(); i2 != service.requests.end(); i2++) {
            optional<S7nodavePlcAddress> plcAddress;
            int bufferSize;
            void *buffer;
            boost::tie(plcAddress, bufferSize, buffer) = *i2;
            requests.push_back(boost::make_tuple(*plcAddress, bufferSize, buffer, requester));
        }
        // Clear list for next requester
        service.requests.clear();
    }

    // Prepare read request
    asynStatus status = asynSuccess;
    asynS7nodaveReadItem *firstItem = NULL;
    asynS7nodaveReadItem *item = NULL;
    for (PollRequestWithRequesterList::iterator i = requests.begin(); i != requests.end(); i++) {
        if (firstItem == NULL) {
            firstItem = static_cast<asynS7nodaveReadItem *>(pasynManager->memMalloc(sizeof(asynS7nodaveReadItem)));
            item = firstItem;
            if (item == NULL) {
                asynPrint(this->myAsynUser, ASYN_TRACE_ERROR, "port \"%s\" poll group \"%s\" S7nodavePollGroup::process Allocation of %d bytes of memory failed.\n", this->portName.c_str(), this->name.c_str(), sizeof(asynS7nodaveReadItem));
                status = asynError;
                break;
            }
        } else {
            asynS7nodaveReadItem *newItem = static_cast<asynS7nodaveReadItem *>(pasynManager->memMalloc(sizeof(asynS7nodaveReadItem)));
            if (newItem == NULL) {
                asynPrint(myAsynUser, ASYN_TRACE_ERROR, "port \"%s\" poll group \"%s\" S7nodavePollGroup::process Allocation of %d bytes of memory failed.\n", this->portName.c_str(), this->name.c_str(), sizeof(asynS7nodaveReadItem));
                status = asynError;
                item = firstItem;
                while (item != NULL) {
                    asynS7nodaveReadItem *nextItem = item->next;
                    pasynManager->memFree(item, sizeof(asynS7nodaveReadItem));
                    item = nextItem;
                }
                firstItem = NULL;
                item = NULL;
                break;
            }
            item->next = newItem;
            item = newItem;
        }
        optional<S7nodavePlcAddress> plcAddress;
        int bufferSize;
        void *buffer;
        boost::tie(plcAddress, bufferSize, buffer, boost::tuples::ignore) = *i;
        item->area = plcAddress->getArea();
        item->areaNumber = plcAddress->getAreaNumber();
        item->startByte = plcAddress->getStartByte();
        item->readBytes = bufferSize;
        item->buffer = buffer;
        item->bytesRead = 0;
        item->next = NULL;
    }

    // Lock port
    bool portLocked = false;
    if (status == asynSuccess) {
        status = pasynManager->lockPort(this->myAsynUser);
        if (status == asynSuccess) {
            portLocked = true;
        }
    }

    // Send read request
    if (status == asynSuccess) {
        status = this->myAsynS7nodave->plcReadMultipleItems(this->myAsynS7nodaveInterface->drvPvt, myAsynUser, firstItem);
    }

    // Unlock port
    if (portLocked) {
        // If unlockPort fails, we just print an error message, because there
        // is no way to recover.
        asynStatus unlockStatus = pasynManager->unlockPort(this->myAsynUser);
        if (unlockStatus != asynSuccess) {
            asynPrint(this->myAsynUser, ASYN_TRACE_ERROR, "port \"%s\" poll group \"%s\" S7nodavePollGroup::process pasynManager->unlockPort %s\n", this->portName.c_str(), this->name.c_str(), this->myAsynUser->errorMessage);
        }
    }

    // Free memory reserved for read items
    item = firstItem;
    while (item != NULL) {
        asynS7nodaveReadItem *nextItem = item->next;
        pasynManager->memFree(item, sizeof(asynS7nodaveReadItem));
        item = nextItem;
    }
    item = NULL;
    firstItem = NULL;

    // Process results. This has to be done always, because the requesters need
    // a chance to free the memory they allocated for the buffers.
    for (PollRequestWithRequesterList::iterator i = requests.begin(); i != requests.end(); i++) {
        int bufferSize;
        void *buffer;
        S7nodavePollRequester *requester;
        boost::tie(boost::tuples::ignore, bufferSize, buffer, requester) = *i;
        requester->processResponse((status == asynSuccess), bufferSize, buffer);
    }
}

// Initialize static attributes
PollGroupMap S7nodavePollGroup::pollGroups;
epicsMutex S7nodavePollGroup::staticMutex;
bool S7nodavePollGroup::isStarted = false;

void s7nodaveConfigurePollGroup(char *portName, char *pollGroupName, double pollingInterval, int priority)
{
    unsigned long unsignedPriority;
    if (priority > 0) {
        unsignedPriority = priority;
    } else {
        unsignedPriority = epicsThreadPriorityMedium;
    }
    S7nodavePollGroup::create(portName, pollGroupName, pollingInterval, unsignedPriority);
}

void s7nodaveStartPollGroups()
{
    S7nodavePollGroup::startPollGroups();
}
