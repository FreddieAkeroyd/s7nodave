#ifndef S7nodaveMbboRecord_h
#define S7nodaveMbboRecord_h

#include <mbboRecord.h>

#include "S7nodaveOutputRecord.h"
#include "s7nodave.h"
#include "s7nodaveAsyn.h"
#include "S7nodaveMultiBinarySupport.h"

/**
 * Device support for mbbo record.
 */
class S7nodaveMbboRecord : public S7nodaveOutputRecord
{
public:
    /**
     * Constructor. The passed record pointer is stored and used by all methods,
     * which need to access record fields.
     */
    S7nodaveMbboRecord(dbCommon *record) :
        S7nodaveOutputRecord(record, mbboRecordType)
    {
    };

protected:
    virtual long interceptInitRecordBeforeValueInit()
    {
        S7nodaveMultiBinarySupport::initMask<mbboRecord>(this->record, this->recordAddress->getPlcAddress().getDataSize());
        return RECORD_STATUS_OK;
    };

    virtual boost::optional<s7nodavePlcDataType> getPlcDataType(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> suggestion)
    {
        boost::optional<s7nodavePlcDataType> defaultType = S7nodaveOutputRecord::getPlcDataType(plcAddress, suggestion);
        return S7nodaveMultiBinarySupport::getPlcDataType(plcAddress, suggestion, *defaultType);
    };

    virtual DBLINK getDeviceAddress() const
    {
        mbboRecord *mbboRec = reinterpret_cast<mbboRecord *>(this->record);
        return mbboRec->out;
    };

    virtual void readFromRecord(void *buffer, int bufferSize) const
    {
        S7nodaveMultiBinarySupport::write<mbboRecord>(this->myAsynUser, this->record, buffer, bufferSize, this->recordAddress->getPlcDataType());
    };

    virtual long writeToRecord(void *buffer, int bufferSize)
    {
        S7nodaveMultiBinarySupport::read<mbboRecord>(this->myAsynUser, this->record, buffer, bufferSize, this->recordAddress->getPlcDataType());
        return RECORD_STATUS_OK;
    };
};

#endif
