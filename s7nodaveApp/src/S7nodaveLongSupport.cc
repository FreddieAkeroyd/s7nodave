#include <string>

#include <alarm.h>
#include <epicsTypes.h>
#include <longinRecord.h>
#include <longoutRecord.h>
#include <recGbl.h>

#include "S7nodaveLongSupport.h"

using boost::optional;
using std::string;

void S7nodaveLongSupport::readInternal(asynUser *pasynUser, dbCommon *record, epicsInt32& val, const void *buffer, int bufferSize, s7nodavePlcDataType plcDataType)
{
    switch (plcDataType) {
    case plcDataTypeBool:
        val = *(static_cast<const unsigned char *>(buffer));
        break;
    case plcDataTypeInt8:
        val = *(static_cast<const epicsInt8 *>(buffer));
        break;
    case plcDataTypeUint8:
        val = *(static_cast<const epicsUInt8 *>(buffer));
        break;
    case plcDataTypeInt16:
        val = *(static_cast<const epicsInt16 *>(buffer));
        break;
    case plcDataTypeUint16:
        val = *(static_cast<const epicsUInt16 *>(buffer));
        break;
    case plcDataTypeInt32:
        val = *(static_cast<const epicsInt32 *>(buffer));
        break;
    default:
        // Data type (float, uint32) is not supported
        recGblSetSevr(record, READ_ALARM, INVALID_ALARM);
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s S7nodaveLongSupport::readInternal Got unexpected PLC data-type.\n", record->name);
        return;
    }
    asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveLongSupport::readInternal read long %d\n", record->name, val);
}

void S7nodaveLongSupport::writeInternal(asynUser *pasynUser, dbCommon *record, epicsInt32 recordVal, void* buffer, int bufferSize, s7nodavePlcDataType plcDataType)
{
    switch (plcDataType) {
    case plcDataTypeBool:
    {
        unsigned char *val = static_cast<unsigned char *>(buffer);
        *val = (recordVal != 0) ? 1 : 0;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveLongSupport::writeInternal writing bit %hhu\n", record->name, *val);
        break;
    }
    case plcDataTypeInt8:
    {
        epicsInt8 *val = static_cast<epicsInt8 *>(buffer);
        *val = recordVal;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveLongSupport::writeInternal writing bit %hhd\n", record->name, *val);
        break;
    }
    case plcDataTypeUint8:
    {
        epicsUInt8 *val = static_cast<epicsUInt8 *>(buffer);
        *val = recordVal;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveLongSupport::writeInternal writing bit %hhu\n", record->name, *val);
        break;
    }
    case plcDataTypeInt16:
    {
        epicsInt16 *val = static_cast<epicsInt16 *>(buffer);
        *val = recordVal;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveLongSupport::writeInternal writing bit %hd\n", record->name, *val);
        break;
    }
    case plcDataTypeUint16:
    {
        epicsUInt16 *val = static_cast<epicsUInt16 *>(buffer);
        *val = recordVal;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveLongSupport::writeInternal writing bit %hu\n", record->name, *val);
        break;
    }
    case plcDataTypeInt32:
    {
        epicsInt32 *val = static_cast<epicsInt32 *>(buffer);
        *val = recordVal;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveLongSupport::writeInternal writing bit %d\n", record->name, *val);
        break;
    }
    default:
        // Data type (float, uint32) is not supported
        recGblSetSevr(record, WRITE_ALARM, INVALID_ALARM);
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s S7nodaveLongSupport::readInternal Got unexpected PLC data-type.\n", record->name);
        break;
    }
}

boost::optional<s7nodavePlcDataType> S7nodaveLongSupport::getPlcDataType(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> userRequestedType, s7nodavePlcDataType defaultType)
{
    bool userRequested;
    s7nodavePlcDataType dataType;
    if (userRequestedType) {
        userRequested = true;
        dataType = *userRequestedType;
    } else {
        userRequested = false;
        dataType = defaultType;
    }
    if (dataType == plcDataTypeUint32 || dataType == plcDataTypeFloat) {
        // These data types are not supported
        if (userRequested) {
            return boost::optional<s7nodavePlcDataType>();
        } else {
            // Suggest int32
            return plcDataTypeInt32;
        }
    } else {
        return dataType;
    }
}
