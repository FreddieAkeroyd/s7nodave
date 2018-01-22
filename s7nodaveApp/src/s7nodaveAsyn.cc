#include <stdlib.h>
#include <string.h>

#include <asynDriver.h>

#include "S7nodaveAaiRecord.h"
#include "S7nodaveAaoRecord.h"
#include "S7nodaveAiRecord.h"
#include "S7nodaveAoRecord.h"
#include "S7nodaveBiRecord.h"
#include "S7nodaveBoRecord.h"
#include "S7nodaveLonginRecord.h"
#include "S7nodaveLongoutRecord.h"
#include "S7nodaveMbbiDirectRecord.h"
#include "S7nodaveMbbiRecord.h"
#include "S7nodaveMbboDirectRecord.h"
#include "S7nodaveMbboRecord.h"
#include "S7nodavePortDriver.h"
#include "S7nodaveRecord.h"
#include "S7nodaveStringinRecord.h"
#include "S7nodaveStringoutRecord.h"
#include "S7nodaveWaveformInRecord.h"
#include "S7nodaveWaveformOutRecord.h"
#include "s7nodave.h"

#include "s7nodaveAsyn.h"

extern "C"{

void s7nodaveConfigureIsoTcpPort(const char *portName, const char* plcHostnameAndPort, const int plcRack, const int plcSlot, const unsigned int priority)
{
    // Divide plcHostname into hostname and port (if specified)
    char *plcHostname = NULL;
    int plcPort = 102;
    const char *colonAndPort = strchr(plcHostnameAndPort, ':');
    if (colonAndPort != NULL) {
        const char *port = ++colonAndPort;
        plcPort = atoi(port);
 //       plcHostname = strndup(plcHostnameAndPort, colonAndPort - plcHostnameAndPort); // strndup not yet on WIN32
        plcHostname = strdup(plcHostnameAndPort);
        plcHostname[colonAndPort - plcHostnameAndPort] = '\0';
    } else {
        plcHostname = strdup(plcHostnameAndPort);
    }

    if (plcHostname == NULL) {
        printf("Cannot register port \"%s\": strdup() failed.\n", portName);
        return;
    }

    // There is no way to unregister a port or interface, thus we know that
    // the memory used by S7nodavePortDriver will never have to be freed.
    S7nodavePortDriver *portDriver = new S7nodavePortDriver(portName, plcHostname, plcPort, plcRack, plcSlot, priority);
    if (portDriver->registerPortDriver() != asynSuccess) {
        // Error message is already generated by registerPortDriver,
        // so we just have to clean up here.
        delete portDriver;
        portDriver = NULL;
    }
    free(plcHostname);
    plcHostname = NULL;
}

long s7nodaveInitRecord(dbCommon *record, s7nodaveRecordType recordType)
{
    S7nodaveRecord *s7nodaveRecord;
    switch (recordType) {
    case aaiRecordType:
        s7nodaveRecord = new S7nodaveAaiRecord(record);
        break;
    case aaoRecordType:
        s7nodaveRecord = new S7nodaveAaoRecord(record);
        break;
    case aiRecordType:
        s7nodaveRecord = new S7nodaveAiRecord(record);
        break;
    case aoRecordType:
        s7nodaveRecord = new S7nodaveAoRecord(record);
        break;
    case biRecordType:
        s7nodaveRecord = new S7nodaveBiRecord(record);
        break;
    case boRecordType:
        s7nodaveRecord = new S7nodaveBoRecord(record);
        break;
    case longinRecordType:
        s7nodaveRecord = new S7nodaveLonginRecord(record);
        break;
    case longoutRecordType:
        s7nodaveRecord = new S7nodaveLongoutRecord(record);
        break;
    case mbbiRecordType:
        s7nodaveRecord = new S7nodaveMbbiRecord(record);
        break;
    case mbboRecordType:
        s7nodaveRecord = new S7nodaveMbboRecord(record);
        break;
    case mbbiDirectRecordType:
        s7nodaveRecord = new S7nodaveMbbiDirectRecord(record);
        break;
    case mbboDirectRecordType:
        s7nodaveRecord = new S7nodaveMbboDirectRecord(record);
        break;
    case stringinRecordType:
        s7nodaveRecord = new S7nodaveStringinRecord(record);
        break;
    case stringoutRecordType:
        s7nodaveRecord = new S7nodaveStringoutRecord(record);
        break;
    case waveformInRecordType:
        s7nodaveRecord = new S7nodaveWaveformInRecord(record);
        break;
    case waveformOutRecordType:
        s7nodaveRecord = new S7nodaveWaveformOutRecord(record);
        break;
    default:
        // Unknown record type
        return RECORD_STATUS_ERROR;
    }

    record->dpvt = s7nodaveRecord;
    return s7nodaveRecord->initRecord();
}

long s7nodaveProcessRecord(dbCommon *record)
{
    if (record->dpvt == NULL) {
        return RECORD_STATUS_ERROR;
    }
    S7nodaveRecord *s7nodaveRecord = static_cast<S7nodaveRecord *>(record->dpvt);
    return s7nodaveRecord->processRecord();
}

long s7nodaveConvertRecord(dbCommon *record, int pass)
{
    if (record->dpvt == NULL) {
        return RECORD_STATUS_ERROR;
    }
    S7nodaveRecord *s7nodaveRecord = static_cast<S7nodaveRecord *>(record->dpvt);
    return s7nodaveRecord->convertRecord(pass);
}

long s7nodaveGetIoIntInfoRecord(int command, dbCommon *record, IOSCANPVT *iopvt)
{
    if (record->dpvt == NULL) {
        return RECORD_STATUS_ERROR;
    }
    S7nodaveRecord *s7nodaveRecord = static_cast<S7nodaveRecord *>(record->dpvt);
    return s7nodaveRecord->getIoIntInfoRecord(command, iopvt);
}

}
