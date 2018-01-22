#include <limits>
#include <string>

#include <alarm.h>
#include <epicsTypes.h>
#include <biRecord.h>
#include <boRecord.h>
#include <recGbl.h>

#include "S7nodaveBinarySupport.h"

using boost::optional;
using std::string;

void S7nodaveBinarySupport::readInternal(asynUser *pasynUser, dbCommon *record, epicsUInt32& rval, const void *buffer, int bufferSize, s7nodavePlcDataType plcDataType)
{
    switch (plcDataType) {
    case plcDataTypeBool:
    {
        const unsigned char val = *(static_cast<const unsigned char *>(buffer));
        rval = (val != 0) ? 1 : 0;
        break;
    }
    case plcDataTypeInt8:
    {
        const epicsInt8 val = *(static_cast<const epicsInt8 *>(buffer));
        rval = (val != 0) ? 1 : 0;
        break;
    }
    case plcDataTypeUint8:
    {
        const epicsUInt8 val = *(static_cast<const epicsUInt8 *>(buffer));
        rval = (val != 0) ? 1 : 0;
        break;
    }
    case plcDataTypeInt16:
    {
        const epicsInt16 val = *(static_cast<const epicsInt16 *>(buffer));
        rval = (val != 0) ? 1 : 0;
        break;
    }
    case plcDataTypeUint16:
    {
        const epicsUInt16 val = *(static_cast<const epicsUInt16 *>(buffer));
        rval = (val != 0) ? 1 : 0;
        break;
    }
    case plcDataTypeInt32:
    {
        const epicsInt32 val = *(static_cast<const epicsInt32 *>(buffer));
        rval = (val != 0) ? 1 : 0;
        break;
    }
    case plcDataTypeUint32:
    {
        const epicsUInt32 val = *(static_cast<const epicsUInt32 *>(buffer));
        rval = (val != 0) ? 1 : 0;
        break;
    }
    case plcDataTypeFloat:
    {
        const epicsFloat32 val = *(static_cast<const epicsFloat32 *>(buffer));
        std::numeric_limits<epicsFloat32> floatLimits;
        if (val != 0.0 && val != floatLimits.quiet_NaN() && val != floatLimits.signaling_NaN() && val != floatLimits.infinity() && val != -floatLimits.infinity()) {
            rval = 1;
        } else {
            rval = 0;
        }
        break;
    }
    }
    asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveBinarySupport::readInternal read boolean %s\n", record->name, (rval != 0) ? "TRUE" : "FALSE");
}

void S7nodaveBinarySupport::writeInternal(asynUser *pasynUser, dbCommon *commonRecord, epicsUInt32 rval, void* buffer, int bufferSize, s7nodavePlcDataType plcDataType)
{
    switch (plcDataType) {
    case plcDataTypeBool:
    {
        unsigned char *val = static_cast<unsigned char *>(buffer);
        *val = (rval != 0) ? 1 : 0;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveBinarySupport::writeInternal writing bit %hhu\n", commonRecord->name, *val);
        break;
    }
    case plcDataTypeInt8:
    {
        epicsInt8 *val = static_cast<epicsInt8 *>(buffer);
        *val = (rval != 0) ? 1 : 0;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveBinarySupport::writeInternal writing int8 %hhd\n", commonRecord->name, *val);
        break;
    }
    case plcDataTypeUint8:
    {
        epicsUInt8 *val = static_cast<epicsUInt8 *>(buffer);
        *val = (rval != 0) ? 1 : 0;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveBinarySupport::writeInternal writing uint8 %hhu\n", commonRecord->name, *val);
        break;
    }
    case plcDataTypeInt16:
    {
        epicsInt16 *val = static_cast<epicsInt16 *>(buffer);
        *val = (rval != 0) ? 1 : 0;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveBinarySupport::writeInternal writing int16 %hd\n", commonRecord->name, *val);
        break;
    }
    case plcDataTypeUint16:
    {
        epicsUInt16 *val = static_cast<epicsUInt16 *>(buffer);
        *val = (rval != 0) ? 1 : 0;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveBinarySupport::writeInternal writing uint16 %hu\n", commonRecord->name, *val);
        break;
    }
    case plcDataTypeInt32:
    {
        epicsInt32 *val = static_cast<epicsInt32 *>(buffer);
        *val = (rval != 0) ? 1 : 0;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveBinarySupport::writeInternal writing int32 %d\n", commonRecord->name, *val);
        break;
    }
    case plcDataTypeUint32:
    {
        epicsUInt32 *val = static_cast<epicsUInt32 *>(buffer);
        *val = (rval != 0) ? 1 : 0;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveBinarySupport::writeInternal writing uint32 %u\n", commonRecord->name, *val);
        break;
    }
    case plcDataTypeFloat:
    {
        epicsFloat32 *val = static_cast<epicsFloat32 *>(buffer);
        *val = (rval != 0) ? 1.0 : 0.0;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveBinarySupport::writeInternal writing float %f\n", commonRecord->name, *val);
        break;
    }
    }
}

boost::optional<s7nodavePlcDataType> S7nodaveBinarySupport::getPlcDataType(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> userRequestedType, s7nodavePlcDataType defaultType)
{
    // All data types are supported
    s7nodavePlcDataType dataType;
    if (userRequestedType) {
        dataType = *userRequestedType;
    } else {
        dataType = defaultType;
    }
    return dataType;
}
