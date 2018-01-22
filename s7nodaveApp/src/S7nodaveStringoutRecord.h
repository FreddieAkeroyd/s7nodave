#ifndef S7nodaveStringoutRecord_h
#define S7nodaveStringoutRecord_h

#include <stringoutRecord.h>
#include <dbFldTypes.h>

#include "S7nodaveOutputRecord.h"
#include "s7nodave.h"
#include "s7nodaveAsyn.h"
#include "S7nodaveStringSupport.h"

/**
 * Device support for stringout record.
 */
class S7nodaveStringoutRecord : public S7nodaveOutputRecord
{
public:
    /**
     * Constructor. The passed record pointer is stored and used by all methods,
     * which need to access record fields.
     */
    S7nodaveStringoutRecord(dbCommon *record) :
        S7nodaveOutputRecord(record, stringoutRecordType)
    {
    };

protected:

    virtual boost::optional<s7nodavePlcDataType> getPlcDataType(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> suggestion)
    {
        return S7nodaveStringSupport::getPlcDataType(plcAddress, suggestion);
    };

    virtual DBLINK getDeviceAddress() const
    {
        stringoutRecord *soRec = reinterpret_cast<stringoutRecord *>(this->record);
        return soRec->out;
    };

    virtual void readFromRecord(void *buffer, int bufferSize) const
    {
        S7nodaveStringSupport::write<stringoutRecord>(this->myAsynUser, this->record, buffer, bufferSize, this->recordAddress->getPlcDataType());
    };

    virtual unsigned long int getIoBufferSizeInBits() const
    {
        return S7nodaveStringSupport::getIoBufferSizeInBits(this->recordAddress->getPlcDataType());
    };

    virtual long writeToRecord(void *buffer, int bufferSize)
    {
        return S7nodaveStringSupport::read<stringoutRecord>(this->myAsynUser, this->record, buffer, bufferSize, this->recordAddress->getPlcDataType());
    };
};

#endif
