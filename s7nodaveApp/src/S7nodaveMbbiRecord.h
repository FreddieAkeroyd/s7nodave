#ifndef S7nodaveMbbiRecord_h
#define S7nodaveMbbiRecord_h

#include <mbbiRecord.h>

#include "S7nodaveInputRecord.h"
#include "s7nodave.h"
#include "s7nodaveAsyn.h"
#include "S7nodaveMultiBinarySupport.h"

/**
 * Device support for mbbi record.
 */
class S7nodaveMbbiRecord : public S7nodaveInputRecord
{
public:
    /**
     * Constructor. The passed record pointer is stored and used by all methods,
     * which need to access record fields.
     */
    S7nodaveMbbiRecord(dbCommon *record) :
        S7nodaveInputRecord(record, mbbiRecordType)
    {
    };

    virtual long initRecord()
    {
        long status;
        status = S7nodaveInputRecord::initRecord();
        if (status == RECORD_STATUS_OK) {
            S7nodaveMultiBinarySupport::initMask<mbbiRecord>(this->record, this->recordAddress->getPlcAddress().getDataSize());
        }
        return status;
    };

protected:
    virtual boost::optional<s7nodavePlcDataType> getPlcDataType(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> suggestion)
    {
        boost::optional<s7nodavePlcDataType> defaultType = S7nodaveInputRecord::getPlcDataType(plcAddress, suggestion);
        return S7nodaveMultiBinarySupport::getPlcDataType(plcAddress, suggestion, *defaultType);
    };

    virtual DBLINK getDeviceAddress() const
    {
        mbbiRecord *mbbiRec = reinterpret_cast<mbbiRecord *>(this->record);
        return mbbiRec->inp;
    };

    virtual long writeToRecord(void *buffer, int bufferSize)
    {
        S7nodaveMultiBinarySupport::read<mbbiRecord>(this->myAsynUser, this->record, buffer, bufferSize, this->recordAddress->getPlcDataType());
        return RECORD_STATUS_OK;
    };
};

#endif
