#ifndef S7nodaveBiRecord_h
#define S7nodaveBiRecord_h

#include <biRecord.h>

#include "S7nodaveInputRecord.h"
#include "s7nodave.h"
#include "s7nodaveAsyn.h"
#include "S7nodaveBinarySupport.h"

/**
 * Device support for ai record.
 */
class S7nodaveBiRecord : public S7nodaveInputRecord
{
public:
    /**
     * Constructor. The passed record pointer is stored and used by all methods,
     * which need to access record fields.
     */
    S7nodaveBiRecord(dbCommon *record) :
        S7nodaveInputRecord(record, biRecordType)
    {
    };

protected:
    virtual boost::optional<s7nodavePlcDataType> getPlcDataType(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> suggestion)
    {
        boost::optional<s7nodavePlcDataType> defaultType = S7nodaveInputRecord::getPlcDataType(plcAddress, suggestion);
        return S7nodaveBinarySupport::getPlcDataType(plcAddress, suggestion, *defaultType);
    };

    virtual DBLINK getDeviceAddress() const
    {
        biRecord *biRec = reinterpret_cast<biRecord *>(this->record);
        return biRec->inp;
    };

    virtual long writeToRecord(void *buffer, int bufferSize)
    {
        S7nodaveBinarySupport::read<biRecord>(this->myAsynUser, this->record, buffer, bufferSize, this->recordAddress->getPlcDataType());
        return RECORD_STATUS_OK;
    };
};

#endif
