#ifndef S7nodaveBoRecord_h
#define S7nodaveBoRecord_h

#include <boRecord.h>

#include "S7nodaveOutputRecord.h"
#include "s7nodave.h"
#include "s7nodaveAsyn.h"
#include "S7nodaveBinarySupport.h"

/**
 * Device support for bo record.
 */
class S7nodaveBoRecord : public S7nodaveOutputRecord
{
public:
    /**
     * Constructor. The passed record pointer is stored and used by all methods,
     * which need to access record fields.
     */
    S7nodaveBoRecord(dbCommon *record) :
        S7nodaveOutputRecord(record, boRecordType)
    {

    };

protected:
    virtual boost::optional<s7nodavePlcDataType> getPlcDataType(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> suggestion)
    {
        boost::optional<s7nodavePlcDataType> defaultType = S7nodaveOutputRecord::getPlcDataType(plcAddress, suggestion);
        return S7nodaveBinarySupport::getPlcDataType(plcAddress, suggestion, *defaultType);
    };

    virtual DBLINK getDeviceAddress() const
    {
        boRecord *boRec = reinterpret_cast<boRecord *>(this->record);
        return boRec->out;
    };

    virtual void readFromRecord(void *buffer, int bufferSize) const
    {
        S7nodaveBinarySupport::write<boRecord>(this->myAsynUser, this->record, buffer, bufferSize, this->recordAddress->getPlcDataType());
    };

    virtual long writeToRecord(void *buffer, int bufferSize)
    {
        S7nodaveBinarySupport::read<boRecord>(this->myAsynUser, this->record, buffer, bufferSize, this->recordAddress->getPlcDataType());
        return RECORD_STATUS_OK;
    };
};

#endif
