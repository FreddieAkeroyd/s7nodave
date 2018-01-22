#include <string>

#include <string.h>

#include <boost/algorithm/string.hpp>

#include <alarm.h>
#include <epicsTypes.h>
#include <recGbl.h>

#include "S7nodaveStringSupport.h"

using boost::optional;
using std::string;

void S7nodaveStringSupport::readInternal(asynUser *pasynUser, dbCommon *record, char *val, const void *buffer, int bufferSize, s7nodavePlcDataType plcDataType)
{
    switch (plcDataType) {
    case plcDataTypeBool:
    {
        const unsigned char *typedBuffer = static_cast<const unsigned char *>(buffer);
        if (*typedBuffer != 0) {
            strncpy(val, "TRUE", MAX_STRING_SIZE);
        } else {
            strncpy(val, "FALSE", MAX_STRING_SIZE);
        }
        // Ensure that string is null-terminated.
        val[MAX_STRING_SIZE - 1] = 0;
        break;
    }
    case plcDataTypeInt8:
    case plcDataTypeUint8:
    {
        const char *typedBuffer = static_cast<const char *>(buffer);
        strncpy(val, typedBuffer, MAX_STRING_SIZE);
        // Ensure that string is null-terminated.
        val[MAX_STRING_SIZE - 1] = 0;
        break;
    }
    default:
        // Data type (float, int16, uint16, int32, uint32) is not supported
        recGblSetSevr(record, READ_ALARM, INVALID_ALARM);
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s S7nodaveStringSupport::readInternal Got unexpected PLC data-type.\n", record->name);
        return;
    }
    asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveStringSupport::readInternal read string \"%s\"\n", record->name, val);
}

void S7nodaveStringSupport::writeInternal(asynUser *pasynUser, dbCommon *record, char* val, void* buffer, int bufferSize, s7nodavePlcDataType plcDataType)
{
    switch (plcDataType) {
    case plcDataTypeBool:
    {
        // We can support the bool (bit) data-type, because there is no
        // ambiguity in how to interpret this kind of data.
        unsigned char *typedBuffer = static_cast<unsigned char *>(buffer);
        string recordVal = val;
        boost::trim(recordVal);
        boost::to_upper(recordVal);
        if (recordVal == "TRUE" || recordVal == "1") {
            *typedBuffer = 1;
        } else {
            *typedBuffer = 0;
        }
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveStringSupport::writeInternal writing bit %hhu\n", record->name, *typedBuffer);
        break;
    }
    case plcDataTypeInt8:
    case plcDataTypeUint8:
    {
        // For characters, we do not care whether the bytes are signed or
        // unsigned. They just interpreted as characters.
        char *typedBuffer = static_cast<char *>(buffer);
        strncpy(typedBuffer, val, MAX_STRING_SIZE);
        // Ensure that string is null-terminated.
        typedBuffer[MAX_STRING_SIZE - 1] = 0;
        asynPrint(pasynUser, ASYN_TRACEIO_DEVICE, "%s S7nodaveStringSupport::readInternal writing string \"%s\"\n", record->name, typedBuffer);
        break;
    }
    default:
        // Data type (float, int16, uint16, int32, uint32) is not supported
        recGblSetSevr(record, WRITE_ALARM, INVALID_ALARM);
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s S7nodaveStringSupport::writeInternal Got unexpected PLC data-type.\n", record->name);
        break;
    }
}

boost::optional<s7nodavePlcDataType> S7nodaveStringSupport::getPlcDataType(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> suggestion)
{
    if (suggestion)
    {
        switch (*suggestion) {
        case plcDataTypeBool:
            return suggestion;
        case plcDataTypeInt8:
        case plcDataTypeUint8:
            return suggestion;
        default:
            return optional<s7nodavePlcDataType>();
        }
    } else {
        switch (plcAddress.getDataSize()) {
        case plcDataSizeBit:
            return plcDataTypeBool;
        case plcDataSizeByte:
            return plcDataTypeInt8;
        default:
            return optional<s7nodavePlcDataType>();
        }
    }
}

unsigned long S7nodaveStringSupport::getIoBufferSizeInBits(s7nodavePlcDataType plcDataType) {
    if (plcDataType == plcDataTypeBool) {
        // Bools are simply stored as a single bit.
        return 8;
    } else {
        // String is special because it has a fixed length of 40 bytes, while
        // the corresponding PLC type is only one byte.
        return MAX_STRING_SIZE * 8;
    }
}
