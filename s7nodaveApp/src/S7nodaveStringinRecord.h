#ifndef S7nodaveStringinRecord_h
#define S7nodaveStringinRecord_h

#include <stringinRecord.h>
#include <dbFldTypes.h>

#include "S7nodaveInputRecord.h"
#include "s7nodave.h"
#include "s7nodaveAsyn.h"
#include "S7nodaveStringSupport.h"

/**
 * Device support for stringin record.
 */
class S7nodaveStringinRecord : public S7nodaveInputRecord
{
public:
    /**
     * Constructor. The passed record pointer is stored and used by all methods,
     * which need to access record fields.
     */
    S7nodaveStringinRecord(dbCommon *record) :
        S7nodaveInputRecord(record, stringinRecordType)
    {
    };

protected:
    virtual boost::optional<s7nodavePlcDataType> getPlcDataType(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> suggestion)
    {
        return S7nodaveStringSupport::getPlcDataType(plcAddress, suggestion);
    };

    virtual DBLINK getDeviceAddress() const
    {
        stringinRecord *siRec = reinterpret_cast<stringinRecord *>(this->record);
        return siRec->inp;
    };

    virtual unsigned long int getIoBufferSizeInBits() const
    {
        return S7nodaveStringSupport::getIoBufferSizeInBits(this->recordAddress->getPlcDataType());
    };

    virtual long writeToRecord(void *buffer, int bufferSize)
    {
        return S7nodaveStringSupport::read<stringinRecord>(this->myAsynUser, this->record, buffer, bufferSize, this->recordAddress->getPlcDataType());
    };
};

#endif
