#include <string>

#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
typedef const char* SOARGTYPE;
#else
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET -1
typedef const void* SOARGTYPE;
#define closesocket close
#endif

#include <asynDriver.h>
#include <epicsGuard.h>

#include "S7nodavePlcAddress.h"
#include "S7nodavePortDriverReadOptimizer.h"

#include "S7nodavePortDriver.h"

typedef S7nodavePortDriverReadOptimizer::ReadItemList ReadItemList;

S7nodavePortDriver::S7nodavePortDriver(const char *portName, const char *plcHostname, const int plcPort, const int plcRack, const int plcSlot, const unsigned int priority)
{
    this->portName = std::string(portName);
    this->plcHostname = std::string(plcHostname);
    this->plcPort = plcPort;
    this->plcRack = plcRack;
    this->plcSlot = plcSlot;
    this->priority = priority;
    this->registered = false;
    this->socketFd = INVALID_SOCKET;

    // Initialize libnodave structures
    this->myDaveInterface = NULL;
    this->myDaveConnection = NULL;

    // Initialize asyn structures
    this->myAsynCommon.report = asynReportStatic;
    this->myAsynCommon.connect = asynConnectStatic;
    this->myAsynCommon.disconnect = asynDisconnectStatic;
    this->myAsynCommonInterface.interfaceType = asynCommonType;
    this->myAsynCommonInterface.pinterface = &this->myAsynCommon;
    this->myAsynCommonInterface.drvPvt = this;
    this->myAsynS7nodave.plcRead = asynPlcReadStatic;
    this->myAsynS7nodave.plcReadMultipleItems = asynPlcReadMultipleItemsStatic;
    this->myAsynS7nodave.plcWrite = asynPlcWriteStatic;
    this->myAsynS7nodave.plcWriteBit = asynPlcWriteBitStatic;
    this->myAsynS7nodaveInterface.interfaceType = asynS7nodaveType;
    this->myAsynS7nodaveInterface.pinterface = &this->myAsynS7nodave;
    this->myAsynS7nodaveInterface.drvPvt = this;
}

S7nodavePortDriver::~S7nodavePortDriver()
{
}

asynStatus S7nodavePortDriver::registerPortDriver()
{
    // Interfaces should only be registered once.
    if (this->registered) {
        return asynSuccess;
    }

    asynStatus status = pasynManager->registerPort(portName.c_str(), ASYN_CANBLOCK, 1, this->priority, 0);
    if (status != asynSuccess) {
        return status;
    }

    status = pasynManager->registerInterface(this->portName.c_str(), &this->myAsynCommonInterface);
    if (status != asynSuccess) {
        return status;
    }

    status = pasynManager->registerInterface(this->portName.c_str(), &this->myAsynS7nodaveInterface);
    if (status != asynSuccess) {
        return status;
    }

    this->registered = true;
    return asynSuccess;
}

void S7nodavePortDriver::asynReport(FILE *fp, int details) {
    // asynReport might be called outside of the port thread, so we
    // cannot really query anything which might depend on the internal state,
    // which basically renders this method useless.
}

/*
 * Opens a TCP socket and connects it to the PLC.
 */
static SOCKET openSocket(std::string hostname, int port) {
    int status;
    if (port <= 0 || port > 65535) {
        printf("Port number %d is outside valid range (1-65535).\n", port);
        return INVALID_SOCKET;
    }
    char portString[6];
    status = snprintf(portString, 6, "%d", port);
    if (status < 0) {
        printf("Cannot convert port number %d to string: snprintf() failed.\n", port);
        return INVALID_SOCKET;
    }
    struct addrinfo hints;
    // Ensure addrinfo structure is initialized with zeros.
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_NUMERICSERV;
    // Allow IPv4 and IPv6.
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *firstAddrinfo;
    status = getaddrinfo(hostname.c_str(), portString, &hints, &firstAddrinfo);
    if (status != 0) {
        printf("Could not get address info for %s:%s: %s\n", hostname.c_str(), portString, gai_strerror(status));
        return INVALID_SOCKET;
    }
    SOCKET socketFd = INVALID_SOCKET;
    struct addrinfo *nextAddrinfo = firstAddrinfo;
    while (socketFd == INVALID_SOCKET && nextAddrinfo != NULL) {
        socketFd = socket(nextAddrinfo->ai_family, nextAddrinfo->ai_socktype, nextAddrinfo->ai_protocol);
        // Continue if socket could not be created
        if (socketFd == INVALID_SOCKET) {
            continue;
        }
        status = connect(socketFd, nextAddrinfo->ai_addr, nextAddrinfo->ai_addrlen);
        if (status == 0) {
            // Connection has been established, so we are done.
            // Now we have to make the I/O non-blocking, because this is
            // required by libnodave.
            //fcntl(socketFd, F_SETFL, O_NONBLOCK);
            // Enable keep-alive packets.
            int socketOptFlag = 1;
            setsockopt(socketFd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<SOARGTYPE>(&socketOptFlag), sizeof(int));
            break;
        }
        // connect() failed, so we close the socket and try with the next address.
        closesocket(socketFd);
        socketFd = INVALID_SOCKET;
        nextAddrinfo = nextAddrinfo->ai_next;
    }
    // Free addrinfo structure
    freeaddrinfo(firstAddrinfo);
    firstAddrinfo = NULL;
    nextAddrinfo = NULL;
    if (socketFd == INVALID_SOCKET) {
        printf("Connection to %s:%s could not be established.\n", hostname.c_str(), portString);
    }
    return socketFd;
}

asynStatus S7nodavePortDriver::asynConnect(asynUser *pasynUser)
{
    {
        // Lock mutex while trying to connect. This avoids double connection
        // attempts when asynConnect is called twice within a short time.
        epicsGuard<epicsMutex> guard(this->connectMutex);
        if (this->socketFd != INVALID_SOCKET) {
            // Already connected
            return asynSuccess;
        }
        this->socketFd = openSocket(this->plcHostname, this->plcPort);
        if (this->socketFd == INVALID_SOCKET) {
            return asynError;
        }

        daveFileDescriptors myDaveFileDescriptors;
#ifdef _WIN32
        myDaveFileDescriptors.rfd = reinterpret_cast<HANDLE>(this->socketFd);
        myDaveFileDescriptors.wfd = reinterpret_cast<HANDLE>(this->socketFd);
#else
        myDaveFileDescriptors.rfd = this->socketFd;
        myDaveFileDescriptors.wfd = this->socketFd;
#endif
        this->myDaveInterface = daveNewInterface(myDaveFileDescriptors, this->portName.c_str(), 0, daveProtoISOTCP, daveSpeed1500k);
        if (this->myDaveInterface == NULL) {
            printf("Could not create dave interface for port %s.\n", this->portName.c_str());
            this->asynDisconnectCleanup(pasynUser);
            return asynError;
        }
        // daveInitAdapter is not needed for ISO-TCP connections, it is still
        // safe to call it, however.
        int status = daveInitAdapter(this->myDaveInterface);
        if (status != 0) {
            printf("Could not initialize dave interface for port %s: daveInitAdapter() failed with status %d.\n", this->portName.c_str(), status);
            this->asynDisconnectCleanup(pasynUser);
            return asynError;
        }
        this->myDaveConnection = daveNewConnection(this->myDaveInterface, 0, this->plcRack, this->plcSlot);
        if (this->myDaveConnection == NULL) {
            printf("Could not create dave connection for port %s.\n", this->portName.c_str());
            this->asynDisconnectCleanup(pasynUser);
            return asynError;
        }
        status = daveConnectPLC(this->myDaveConnection);
        if (status != 0) {
            printf("Could not connect to PLC for port %s: daveConnectPLC() failed with status code %d.\n", this->portName.c_str(), status);
            this->asynDisconnectCleanup(pasynUser);
            return asynError;
        }
    }

    // We send the connect notification without holding the lock to avoid
    // a dead-lock situation.
    pasynManager->exceptionConnect(pasynUser);

    return asynSuccess;
}

asynStatus S7nodavePortDriver::asynDisconnect(asynUser *pasynUser)
{
    // Lock mutex while disconnecting. This avoids interference effects between
    // asynConnect and asynDisconnect.
    {
        epicsGuard<epicsMutex> guard(this->connectMutex);
        if (this->socketFd == INVALID_SOCKET) {
            // Connection was already cleaned up.
            // We return directly in order to avoid calling
            // exceptionDisconnect again.
            return asynSuccess;
        }
        this->asynDisconnectCleanup(pasynUser);
    }

    // We send the disconnect notification without holding the lock to avoid
    // a dead-lock situation.
    pasynManager->exceptionDisconnect(pasynUser);

    return asynSuccess;
}

asynStatus S7nodavePortDriver::asynDisconnectCleanup(asynUser *pasynUser)
{
    // Lock mutex while disconnecting. This avoids interference effects between
    // asynConnect and asynDisconnect.
    epicsGuard<epicsMutex> guard(this->connectMutex);
    if (this->myDaveConnection != NULL) {
        daveDisconnectPLC(this->myDaveConnection);
        daveFree(this->myDaveConnection);
        this->myDaveConnection = NULL;
    }
    if (this->myDaveInterface != NULL) {
        daveDisconnectAdapter(this->myDaveInterface);
        daveFree(this->myDaveInterface);
        this->myDaveInterface = NULL;
    }
    if (this->socketFd != INVALID_SOCKET) {
        closesocket(this->socketFd);
        this->socketFd = INVALID_SOCKET;
    }
    return asynSuccess;
}

/*
 * Converts to the area values libnodave expects.
 */
static int areaToDaveArea(s7nodavePlcArea area)
{
    switch(area) {
    case plcAreaDb:
        return daveDB;
    case plcAreaFlags:
        return daveFlags;
    case plcAreaInputs:
        return daveInputs;
    case plcAreaOutputs:
        return daveOutputs;
    case plcAreaTimer:
        return daveTimer;
    case plcAreaCounter:
        return daveCounter;
    default:
        return -1;
    }
}

/*
 * Checks the trace I/O mask for the ASYN_TRACEIO_DRIVER bit.
 */
static bool shouldTraceIO(asynUser *pasynUser)
{
    return (pasynTrace->getTraceMask(pasynUser) & ASYN_TRACEIO_DRIVER) != 0;
}

asynStatus S7nodavePortDriver::asynPlcRead(asynUser *pasynUser, s7nodavePlcArea area, int areaNumber, int startByte, int readLength, void *buffer, int *bytesRead)
{
    if (this->myDaveConnection == NULL) {
        return asynDisconnected;
    }
    int status = daveReadManyBytes(this->myDaveConnection, areaToDaveArea(area), areaNumber, startByte, readLength, buffer);
    if (status == daveResOK) {
        *bytesRead = readLength;
        // Building a string from the PLC address is relatively expensive, so
        // only do it if needed.
        if (shouldTraceIO(pasynUser)) {
            asynPrintIO(pasynUser, ASYN_TRACEIO_DRIVER, static_cast<char *>(buffer), readLength, "%s S7nodavePortDriver::asynPlcRead read %d bytes of data from PLC address %s: ", this->portName.c_str(), readLength, S7nodavePlcAddress::create(area, areaNumber, plcDataSizeByte, startByte, 0)->toString().c_str());
        }
        return asynSuccess;
    } else if (status == daveResShortPacket || status == daveResTimeout) {
        // Connection might be broken. We should close the connection,
        // so that asynManager will try to reconnect.
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s error while reading %d bytes from PLC address %s, disconnecting...\n", this->portName.c_str(), readLength, S7nodavePlcAddress::create(area, areaNumber, plcDataSizeByte, startByte, 0)->toString().c_str());
        this->asynDisconnect(pasynUser);
        return asynError;
    } else {
        // Other error
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s error while reading %d bytes from PLC address %s.\n", this->portName.c_str(), readLength, S7nodavePlcAddress::create(area, areaNumber, plcDataSizeByte, startByte, 0)->toString().c_str());
        return asynError;
    }
}

/*
 * Sets the bytesRead values in a chained read item structure to zero.
 */
static void setAllBytesReadToZero(asynS7nodaveReadItem *item) {
    while (item != NULL) {
        item->bytesRead = 0;
        item = item->next;
    }
}

/*
 * Copies the result of a (multiple item) read request to the buffers of the
 * corresponding read requests.
 */
static void processPlcReadResult(asynUser *pasynUser, const char *portName, const daveResultSet& resultSet, ReadItemList& correspondingItems) {
    int pos = 0;
    ReadItemList::iterator it = correspondingItems.begin();
    while (it != correspondingItems.end()) {
        if (pos >= resultSet.numResults) {
            // We did not get a response for the item.
            (*it)->setBytesRead(0);
            // We do not have to increase pos, because bigger values of pos will
            // be invalid as well. Therefore, we just increment the iterator.
            it++;
            continue;
        }
        daveResult result = resultSet.results[pos];
        if (result.error == daveResOK) {
            // Set bytes read and copy buffer
            int bytesRead = std::min(result.length, (*it)->getReadBytes());
            (*it)->setBytesRead(bytesRead);
            memcpy((*it)->getBuffer(), result.bytes, bytesRead);
            if (shouldTraceIO(pasynUser)) {
                asynPrintIO(pasynUser, ASYN_TRACEIO_DRIVER, static_cast<char *>((*it)->getBuffer()), bytesRead, "%s S7nodavePortDriver::asynPlcReadMultipleItems read %d bytes of data from PLC address %s: ", portName, bytesRead, S7nodavePlcAddress::create((*it)->getArea(), (*it)->getAreaNumber(), plcDataSizeByte, (*it)->getStartByte(), 0)->toString().c_str());
            }
        } else {
            // There is a problem with the response for this item.
            (*it)->setBytesRead(0);
        }
        // Increment position counter and iterator
        pos++;
        it++;
    }
}

/*
 * Sends several read item to the PLC. Takes care of splitting the items into
 * several requests if they are too big.
 */
static int sendRequestToPlc(asynUser *pasynUser, const char *portName, daveConnection *myDaveConnection, ReadItemList items) {
    // The maxium payload size is the size of a PDU minus the 14 header bytes.
    // These 14 bytes also include the command and item count. In addition to
    // that, 4 bytes have to be reserved for the header of each requested item.
    unsigned int maxSizeOfPayload = myDaveConnection->maxPDUlength - 14;
    // The PLCs can only handle so many items that the request size does not
    // exceed the maximum PDU size. The header (including the read command and
    // item count) occupies 12 bytes. Each read item occupies another 12 bytes.
    unsigned int maxNumberOfItemsPerRequest = (myDaveConnection->maxPDUlength - 12) / 12;
    unsigned int itemsLeft = maxNumberOfItemsPerRequest;
    unsigned int bytesLeft = maxSizeOfPayload;
    bool haveSentItemInThisRun = false;
    ReadItemList itemsInPDU;
    int status = daveResOK;
    davePDU pdu;
    daveResultSet resultSet;
    davePrepareReadRequest(myDaveConnection, &pdu);
    ReadItemList::iterator i = items.begin();
    while (!items.empty() || !itemsInPDU.empty()) {
        if (itemsLeft <= 0 || bytesLeft < 6 || (i == items.end() && !haveSentItemInThisRun) || items.empty()) {
            // There are two reasons, why this block is executed:
            // 1) There is no space left in the PDU. This might be because the
            //    request would grow too large (itemsLeft == 0) or because the
            //    data in the response would grow too large (bytesLeft < 6).
            //    The limit of 6 bytes is because the smallest item (one or two
            //    bytes read) occupies 6 bytes.
            // 2) We already went through the list once without sending an item.
            //    Probably all remaining items are too big to be included in the
            //    current PDU. We have to send the PDU now, in order to avoid an
            //    infinite loop.
            // 3) We have processed all items. In this case we have to send the
            //    last request.
            status = daveExecReadRequest(myDaveConnection, &pdu, &resultSet);
            if (status == daveResOK) {
                processPlcReadResult(pasynUser, portName, resultSet, itemsInPDU);
                daveFreeResults(&resultSet);
            } else {
                // Make sure the bytes read property of the remaining items is
                // set to zero.
                items.splice(items.end(), itemsInPDU);
                for (ReadItemList::iterator i2 = items.begin(); i2 != items.end(); i2++) {
                    (*i2)->setBytesRead(0);
                }
                if (status == daveResShortPacket || status == daveResTimeout) {
                    // Connection might be broken. We should close the
                    // connection, so that asynManager will try to reconnect.
                    // We cannot disconnect here, but the calling routing will
                    // disconnect, when it sees the status code.
                    asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s error while reading from PLC, disconnecting...\n", portName);
                } else {
                    asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s error while reading from PLC.\n", portName);
                }
                return status;
            }
            // We have to prepare the headers again, because they were
            // changed by the last read.
            davePrepareReadRequest(myDaveConnection, &pdu);
            bytesLeft = maxSizeOfPayload;
            itemsLeft = maxNumberOfItemsPerRequest;
            itemsInPDU.clear();
            haveSentItemInThisRun = true;
            if (items.empty()) {
                // We have sent the last PDU, so we can return.
                return status;
            }
        }
        if (i == items.end()) {
            // We have reached the end of the list, but it is not empty.
            // Therefore we start from the beginning of the remaining list.
            // We have to set haveSentItemInThisRun to false in order to avoid
            // an infinite loop.
            haveSentItemInThisRun = false;
            i = items.begin();
        }
        int readBytes = (*i)->getReadBytes();
        // When reading an odd number of bytes, the PLC adds a padding byte
        // before the next item. This means that when reading an odd number of
        // bytes, we actually need space for one byte more. Such a padding byte
        // is not added for the last item in the list of items, but typically
        // this will not help because it would only make a difference if
        // the maximum PDU size was odd. For this reason, we act like any item
        // with an odd number of bytes (even the last item in the list) needs
        // space for an extra byte.
        int effectiveReadBytes = (readBytes % 2 == 0) ? readBytes : (readBytes + 1);
        // We also have to include the extra four bytes that the header
        // associated with each item occupies in the response.
        effectiveReadBytes += 4;
        if (readBytes == 0) {
            // If there are no bytes to be read, we can simply skip the
            // item.
            (*i)->setBytesRead(0);
            i = items.erase(i);
        } else if (static_cast<unsigned int>(effectiveReadBytes) <= bytesLeft) {
            // The item fits in the PDU.
            daveAddVarToReadRequest(&pdu, areaToDaveArea((*i)->getArea()), (*i)->getAreaNumber(), (*i)->getStartByte(), readBytes);
            bytesLeft -= effectiveReadBytes;
            itemsLeft--;
            // Move item to list itemsInPDU and proceed with next one
            itemsInPDU.splice(itemsInPDU.end(), items, i++);
        } else {
            // Item is to big for the remaining space in the PDU. We can
            // continue to look for an item that is smaller. However, if the
            // PDU is still empty, the single item is just to big for a single
            // PDU. In this case we use daveReadManyBytes, which will
            // automatically split the request.
            if (bytesLeft == maxSizeOfPayload) {
                status = daveReadManyBytes(myDaveConnection, areaToDaveArea((*i)->getArea()), (*i)->getAreaNumber(), (*i)->getStartByte(), readBytes, (*i)->getBuffer());
                if (status != daveResOK) {
                    return status;
                }
                // Remove item from list and continue with next item
                i = items.erase(i);
                // We have to prepare the headers again, because they were
                // changed by the last read.
                davePrepareReadRequest(myDaveConnection, &pdu);
                // This branch of the if statement is only used when no items
                // have been added to the PDU, so we do not have to reset
                // bytesLeft, itemsLeft, and itemsInPDU.
            } else {
                // We try the next item.
                i++;
            }
        }
    }
    return status;
}

asynStatus S7nodavePortDriver::asynPlcReadMultipleItems(asynUser *pasynUser, asynS7nodaveReadItem *firstItem)
{
    if (this->myDaveConnection == NULL) {
        setAllBytesReadToZero(firstItem);
        return asynDisconnected;
    }

    // The maxium payload size is the size of a PDU minus the 14 header bytes.
    // In addition to that, 4 bytes have to be reserved for the header of each
    // requested item.
    unsigned int maxSizeOfPayload = this->myDaveConnection->maxPDUlength - 14;

    // Optimize the number of read requests
    ReadItemList readItems = S7nodavePortDriverReadOptimizer::createOptimizedList(firstItem, maxSizeOfPayload);

    // Send requests to PLC and process responses.
    int status = sendRequestToPlc(pasynUser, this->portName.c_str(), this->myDaveConnection, readItems);
    if (status == daveResShortPacket || status == daveResTimeout) {
        this->asynDisconnect(pasynUser);
    }

    // Dispatch the results from the joined requests to the original individual
    // requests and free the allocated memory. This method has to be called,
    // even if there was an error, in order to avoid a memory leak.
    S7nodavePortDriverReadOptimizer::dispatchResults(readItems);

    // Check status code
    if (status == daveResOK) {
        return asynSuccess;
    } else {
        return asynError;
    }
}

asynStatus S7nodavePortDriver::asynPlcWrite(asynUser *pasynUser, s7nodavePlcArea area, int areaNumber, int startByte, int writeLength, void *buffer, int *bytesWritten)
{
    if (this->myDaveConnection == NULL) {
        return asynDisconnected;
    }
    int status = daveWriteManyBytes(this->myDaveConnection, areaToDaveArea(area), areaNumber, startByte, writeLength, buffer);
    if (status == daveResOK) {
        *bytesWritten = writeLength;
        if (shouldTraceIO(pasynUser)) {
            asynPrintIO(pasynUser, ASYN_TRACEIO_DRIVER, static_cast<char *>(buffer), writeLength, "%s S7nodavePortDriver::asynPlcRead write %d bytes of data to PLC address %s: ", this->portName.c_str(), writeLength, S7nodavePlcAddress::create(area, areaNumber, plcDataSizeByte, startByte, 0)->toString().c_str());
        }
        return asynSuccess;
    } else if (status == daveResShortPacket || status == daveResTimeout) {
        // Connection might be broken. We should close the connection,
        // so that asynManager will try to reconnect.
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s error while writing %d bytes to PLC address %s, disconnecting...\n", this->portName.c_str(), writeLength, S7nodavePlcAddress::create(area, areaNumber, plcDataSizeByte, startByte, 0)->toString().c_str());
        this->asynDisconnect(pasynUser);
        return asynError;
    } else {
        // Other error
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s error while writing %d bytes to PLC address %s.\n", this->portName.c_str(), writeLength, S7nodavePlcAddress::create(area, areaNumber, plcDataSizeByte, startByte, 0)->toString().c_str());
        return asynError;
    }
}
asynStatus S7nodavePortDriver::asynPlcWriteBit(asynUser *pasynUser, s7nodavePlcArea area, int areaNumber, int startByte, int startBit, unsigned char bitValue)
{
    if (this->myDaveConnection == NULL) {
        return asynDisconnected;
    }
    int status;
    if (bitValue) {
        status = daveSetBit(this->myDaveConnection, areaToDaveArea(area), areaNumber, startByte, startBit);
    } else {
        status = daveClrBit(this->myDaveConnection, areaToDaveArea(area), areaNumber, startByte, startBit);
    }
    if (status == daveResOK) {
        if (shouldTraceIO(pasynUser)) {
            asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s wrote bit %d to PLC address %s.\n", this->portName.c_str(), (bitValue) ? 1 : 0, S7nodavePlcAddress::create(area, areaNumber, plcDataSizeBit, startByte, startBit)->toString().c_str());
        }
        return asynSuccess;
    } else if (status == daveResShortPacket || status == daveResTimeout) {
        // Connection might be broken. We should close the connection,
        // so that asynManager will try to reconnect.
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s error while writing bit %d to PLC address %s, disconnecting...\n", this->portName.c_str(), (bitValue) ? 1 : 0, S7nodavePlcAddress::create(area, areaNumber, plcDataSizeBit, startByte, startBit)->toString().c_str());
        this->asynDisconnect(pasynUser);
        return asynError;
    } else {
        // Other error
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s error while writing bit %d to PLC address %s.\n", this->portName.c_str(), (bitValue) ? 1 : 0, S7nodavePlcAddress::create(area, areaNumber, plcDataSizeBit, startByte, startBit)->toString().c_str());
        return asynError;
    }
}

void S7nodavePortDriver::asynReportStatic(void *drvPvt, FILE *fp, int details)
{
    S7nodavePortDriver *driver = static_cast<S7nodavePortDriver *>(drvPvt);
    driver->asynReport(fp, details);
}

asynStatus S7nodavePortDriver::asynConnectStatic(void *drvPvt, asynUser *pasynUser)
{
    S7nodavePortDriver *driver = static_cast<S7nodavePortDriver *>(drvPvt);
    return driver->asynConnect(pasynUser);
}

asynStatus S7nodavePortDriver::asynDisconnectStatic(void *drvPvt, asynUser *pasynUser)
{
    S7nodavePortDriver *driver = static_cast<S7nodavePortDriver *>(drvPvt);
    return driver->asynDisconnect(pasynUser);
}

asynStatus S7nodavePortDriver::asynPlcReadStatic(void *drvPvt, asynUser *pasynUser, s7nodavePlcArea area, int areaNumber, int startByte, int readLength, void *buffer, int *bytesRead)
{
    S7nodavePortDriver *driver = static_cast<S7nodavePortDriver *>(drvPvt);
    return driver->asynPlcRead(pasynUser, area, areaNumber, startByte, readLength, buffer, bytesRead);
}

asynStatus S7nodavePortDriver::asynPlcReadMultipleItemsStatic(void *drvPvt, asynUser *pasynUser, asynS7nodaveReadItem *firstItem)
{
    S7nodavePortDriver *driver = static_cast<S7nodavePortDriver *>(drvPvt);
    return driver->asynPlcReadMultipleItems(pasynUser, firstItem);
}

asynStatus S7nodavePortDriver::asynPlcWriteStatic(void *drvPvt, asynUser *pasynUser, s7nodavePlcArea area, int areaNumber, int startByte, int writeLength, void *buffer, int *bytesWritten)
{
    S7nodavePortDriver *driver = static_cast<S7nodavePortDriver *>(drvPvt);
    return driver->asynPlcWrite(pasynUser, area, areaNumber, startByte, writeLength, buffer, bytesWritten);
}

asynStatus S7nodavePortDriver::asynPlcWriteBitStatic(void *drvPvt, asynUser *pasynUser, s7nodavePlcArea area, int areaNumber, int startByte, int startBit, unsigned char bitValue)
{
    S7nodavePortDriver *driver = static_cast<S7nodavePortDriver *>(drvPvt);
    return driver->asynPlcWriteBit(pasynUser, area, areaNumber, startByte, startBit, bitValue);
}

