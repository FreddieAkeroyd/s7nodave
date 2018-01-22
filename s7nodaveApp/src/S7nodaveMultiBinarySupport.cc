#include <limits>
#include <string>

#include <alarm.h>
#include <epicsTypes.h>
#include <mbbiRecord.h>
#include <mbbiDirectRecord.h>
#include <mbboRecord.h>
#include <mbboDirectRecord.h>
#include <recGbl.h>

#include "S7nodavePlcAddress.h"

#include "S7nodaveMultiBinarySupport.h"

using boost::optional;
using std::string;

static void writeCommon(asynUser *pasynUser, dbCommon *record, epicsUInt32 rval, void *buffer, s7nodavePlcDataType plcDataType)
{
    switch (plcDataType) {
    case plcDataTypeInt8:
    case plcDataTypeUint8:
    {
        epicsUInt8 *val = static_cast<epicsUInt8 *>(buffer);
        *val = rval & 0xff;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveMultiBinarySupport::writeCommon writing byte %u\n", record->name, *val);
        break;
    }
    case plcDataTypeInt16:
    case plcDataTypeUint16:
    {
        epicsUInt16 *val = static_cast<epicsUInt16 *>(buffer);
        *val = rval & 0xffff;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveMultiBinarySupport::writeCommon writing word %u\n", record->name, *val);
        break;
    }
    case plcDataTypeInt32:
    case plcDataTypeUint32:
    {
        epicsUInt32 *val = static_cast<epicsUInt32 *>(buffer);
        *val = rval;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveMultiBinarySupport::writeCommon writing byte %u\n", record->name, *val);
        break;
    }
    default:
        // Unsupported type, that should never happen
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s S7nodaveMultiBinarySupport::writeCommon Got unexpected PLC data-type.\n", record->name);
        return;
    }
}

template <>
void S7nodaveMultiBinarySupport::write<mbboRecord>(asynUser *pasynUser, dbCommon *record, void* buffer, int bufferSize, s7nodavePlcDataType plcDataType)
{
    mbboRecord *typedRecord = reinterpret_cast<mbboRecord *>(record);
    epicsUInt32 mask;
    epicsUInt32 rval;
    rval = typedRecord->val;
    mask = typedRecord->mask;
    mbboRecord *r = typedRecord;
    epicsUInt32 *vals[] = {&r->zrvl, &r->onvl, &r->twvl, &r->thvl, &r->frvl, &r->fvvl, &r->sxvl, &r->svvl, &r->eivl, &r->nivl, &r->tevl, &r->elvl, &r->tvvl, &r->ttvl, &r->ftvl, &r->ffvl};
    for (int i = 0; i < 16; i++) {
        if (*vals[i] != 0) {
            // If at least one value is defined, use RVAL
            rval = typedRecord->rval;
            if (mask != 0) {
                rval &= mask;
            }
            break;
        }
    }
    writeCommon(pasynUser, record, rval, buffer, plcDataType);
};

template <>
void S7nodaveMultiBinarySupport::write<mbboDirectRecord>(asynUser *pasynUser, dbCommon *record, void* buffer, int bufferSize, s7nodavePlcDataType plcDataType)
{
    mbboDirectRecord *typedRecord = reinterpret_cast<mbboDirectRecord *>(record);
    epicsUInt32 mask;
    epicsUInt32 rval;
    rval = typedRecord->rval;
    mask = typedRecord->mask;
    if (mask != 0) {
        // Use VAL instead of RVAL
        rval = typedRecord->val;
        rval &= mask;
    }
    writeCommon(pasynUser, record, rval, buffer, plcDataType);
};

void S7nodaveMultiBinarySupport::initMaskInternal(dbCommon *record, epicsUInt32& mask, epicsUInt16& nobt, epicsUInt16 shft, s7nodavePlcDataSize plcDataSize) {
    int dataSizeInBits = S7nodavePlcAddress::dataSizeInBits(plcDataSize);
    if (nobt > dataSizeInBits) {
        nobt = 0;
        mask = 0;
    }
    if (shft > 0) {
        mask <<= shft;
    }
}

void S7nodaveMultiBinarySupport::readInternal(asynUser *pasynUser, dbCommon *record, epicsUInt32& rval, epicsUInt32 mask, const void *buffer, int bufferSize, s7nodavePlcDataType plcDataType)
{
    // We treat all numbers as unsigned, because we are only interested in the
    // bits.
    switch (plcDataType) {
    case plcDataTypeInt8:
    case plcDataTypeUint8:
        rval = *(static_cast<const epicsUInt8 *>(buffer));
        break;
    case plcDataTypeInt16:
    case plcDataTypeUint16:
        rval = *(static_cast<const epicsUInt16 *>(buffer));
        break;
    case plcDataTypeInt32:
    case plcDataTypeUint32:
        rval = *(static_cast<const epicsUInt32 *>(buffer));
        break;
    default:
        // Unsupported type, should never happen
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s S7nodaveMultiBinarySupport::readInternal Got unexpected PLC data-type.\n", record->name);
        return;
    }
    if (mask != 0) {
        rval &= mask;
    }
    asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveMultiBinarySupport::readInternal read long %u\n", record->name, rval);
}

boost::optional<s7nodavePlcDataType> S7nodaveMultiBinarySupport::getPlcDataType(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> userRequestedType, s7nodavePlcDataType defaultType)
{
    s7nodavePlcDataType dataType;
    if (userRequestedType) {
        dataType = *userRequestedType;
    } else {
        dataType = defaultType;
    }
    if (dataType == plcDataTypeBool) {
        // bool is not supported
        return optional<s7nodavePlcDataType>();
    } else if (dataType == plcDataTypeFloat) {
        if (userRequestedType) {
            // float is not supported
            return optional<s7nodavePlcDataType>();
        } else {
            // int32 has the same length as float
            return plcDataTypeUint32;
        }
    }
    return dataType;
}
