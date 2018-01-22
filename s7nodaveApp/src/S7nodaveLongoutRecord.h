#ifndef S7nodaveLongoutRecord_h
#define S7nodaveLongoutRecord_h

#include <longoutRecord.h>

#include "S7nodaveOutputRecord.h"
#include "s7nodave.h"
#include "s7nodaveAsyn.h"
#include "S7nodaveLongSupport.h"

/**
 * Device support for longout record.
 */
class S7nodaveLongoutRecord : public S7nodaveOutputRecord
{
public:
    /**
     * Constructor. The passed record pointer is stored and used by all methods,
     * which need to access record fields.
     */
    S7nodaveLongoutRecord(dbCommon *record) :
        S7nodaveOutputRecord(record, longoutRecordType)
    {
    };

protected:
    virtual boost::optional<s7nodavePlcDataType> getPlcDataType(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> suggestion)
    {
        boost::optional<s7nodavePlcDataType> defaultType = S7nodaveOutputRecord::getPlcDataType(plcAddress, suggestion);
        return S7nodaveLongSupport::getPlcDataType(plcAddress, suggestion, *defaultType);
    };

    virtual DBLINK getDeviceAddress() const
    {
        longoutRecord *loRec = reinterpret_cast<longoutRecord *>(this->record);
        return loRec->out;
    };

    virtual void readFromRecord(void *buffer, int bufferSize) const
    {
        S7nodaveLongSupport::write<longoutRecord>(this->myAsynUser, this->record, buffer, bufferSize, this->recordAddress->getPlcDataType());
    };

    virtual long writeToRecord(void *buffer, int bufferSize)
    {
        S7nodaveLongSupport::read<longoutRecord>(this->myAsynUser, this->record, buffer, bufferSize, this->recordAddress->getPlcDataType());
        return RECORD_STATUS_OK;
    };
};

#endif
