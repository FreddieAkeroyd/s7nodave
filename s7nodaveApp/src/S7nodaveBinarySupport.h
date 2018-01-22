#ifndef S7nodaveBinarySupport_h
#define S7nodaveBinarySupport_h

#include<boost/optional/optional.hpp>

#include <dbCommon.h>

#include "s7nodave.h"
#include "S7nodavePlcAddress.h"
#include "S7nodaveRecordAddress.h"
#include "s7nodaveAsyn.h"

/**
 * This class provides functions that are common to bi and bo records.
 */
class S7nodaveBinarySupport
{
public:
    /**
     * Reads data from buffer into the RVAL field of the record. This function
     * assumes that the buffer already contains the right number of elements
     * and has the correct (host architecture) byte order.
     */
    template <class recordType>
    static long read(asynUser *pasynUser, dbCommon *record, const void *buffer, int bufferSize, s7nodavePlcDataType plcDataType)
    {
        recordType *typedRecord = reinterpret_cast<recordType *>(record);
        readInternal(pasynUser, record, typedRecord->rval, buffer, bufferSize, plcDataType);
        return RECORD_STATUS_OK;
    };

    /**
     * Writes data from the RVAL record field into the buffer. This function
     * assumes that the buffer has sufficient space for the value and writes the
     * value in host architecture byte order.
     */
    template <class recordType>
    static void write(asynUser *pasynUser, dbCommon *record, void* buffer, int bufferSize, s7nodavePlcDataType plcDataType)
    {
        recordType *typedRecord = reinterpret_cast<recordType *>(record);
        writeInternal(pasynUser, record, typedRecord->rval, buffer, bufferSize, plcDataType);
    };

    /**
     * Returns the PLC data-type used for the specified address. If the address
     * string requests a PLC data-type that is not supported by this record
     * type, an empty result is returned.
     */
    static boost::optional<s7nodavePlcDataType> getPlcDataType(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> userRequestedType, s7nodavePlcDataType defaultType);

private:
    /**
     * The constructor is private because all public methods of this class are
     * static.
     */
    S7nodaveBinarySupport() {};

    static void readInternal(asynUser *pasynUser, dbCommon *record, epicsUInt32& rval, const void *buffer, int bufferSize, s7nodavePlcDataType plcDataType);

    static void writeInternal(asynUser *pasynUser, dbCommon *record, epicsUInt32 rval, void* buffer, int bufferSize, s7nodavePlcDataType plcDataType);
};

#endif
