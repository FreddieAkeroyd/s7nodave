#ifndef S7nodaveArraySupport_h
#define S7nodaveArraySupport_h

#include<boost/optional/optional.hpp>

#include <dbCommon.h>
#include <epicsTypes.h>

#include "S7nodavePlcAddress.h"
#include "S7nodaveRecordAddress.h"
#include "s7nodaveAsyn.h"

/**
 * This class provides functions that are common to aai, aao and waveform
 * records.
 */
class S7nodaveArraySupport
{
public:
    /**
     * Reads data from buffer into the record field BPTR. This function assumes
     * that the buffer already contains the right number of elements and has the
     * correct (host architecture) byte order.
     */
    template <class recordType>
    static long read(asynUser *pasynUser, dbCommon *record, const void *buffer, int bufferSize, s7nodavePlcDataType plcDataType)
    {
        recordType *typedRecord = reinterpret_cast<recordType *>(record);
        return readInternal(pasynUser, record, typedRecord->bptr, typedRecord->nelm, typedRecord->ftvl, typedRecord->nord, buffer, bufferSize, plcDataType);
    };

    /**
     * Writes data from the record field BPTR into the buffer. This function
     * assumes that the buffer has sufficient space for the value and writes the
     * value in host architecture byte order.
     */
    template <class recordType>
    static void write(asynUser *pasynUser, dbCommon *record, void* buffer, int bufferSize, s7nodavePlcDataType plcDataType)
    {
        recordType *typedRecord = reinterpret_cast<recordType *>(record);
        writeInternal(pasynUser, record, typedRecord->bptr, typedRecord->nelm, typedRecord->ftvl, typedRecord->nord, buffer, bufferSize, plcDataType);
    };

    /**
     * Returns the PLC data-type used for the specified address. If the address
     * string requests a PLC data-type that is not supported by this record
     * type, an empty result is returned. This method is only used for input
     * records.
     */
    static boost::optional<s7nodavePlcDataType> getPlcDataTypeIn(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> suggestion, epicsEnum16 ftvl);

    /**
     * Returns the PLC data-type used for the specified address. If the address
     * string requests a PLC data-type that is not supported by this record
     * type, an empty result is returned. This method is only used for output
     * records.
     */
    static boost::optional<s7nodavePlcDataType> getPlcDataTypeOut(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> suggestion, epicsEnum16 ftvl);

private:
    /**
     * The constructor is private because all public methods of this class are
     * static.
     */
    S7nodaveArraySupport() {};

    static long readInternal(asynUser *pasynUser, dbCommon *record, void *bptr, epicsUInt32 nelm, epicsEnum16 ftvl, epicsUInt32& nord, const void *buffer, int bufferSize, s7nodavePlcDataType plcDataType);

    static void writeInternal(asynUser *pasynUser, dbCommon *record, void *bptr, epicsUInt32 nelm, epicsEnum16 ftvl, epicsUInt32& nord, void* buffer, int bufferSize, s7nodavePlcDataType plcDataType);
};

#endif
