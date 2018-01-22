#ifndef S7nodaveLonginRecord_h
#define S7nodaveLonginRecord_h

#include <longinRecord.h>

#include "S7nodaveInputRecord.h"
#include "S7nodaveLongSupport.h"
#include "s7nodave.h"
#include "s7nodaveAsyn.h"

/**
 * Device support for longin record.
 */
class S7nodaveLonginRecord : public S7nodaveInputRecord
{
public:
    /**
     * Constructor. The passed record pointer is stored and used by all methods,
     * which need to access record fields.
     */
    S7nodaveLonginRecord(dbCommon *record) :
        S7nodaveInputRecord(record, longinRecordType)
    {
    };

protected:
    virtual boost::optional<s7nodavePlcDataType> getPlcDataType(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> suggestion)
    {
        boost::optional<s7nodavePlcDataType> defaultType = S7nodaveInputRecord::getPlcDataType(plcAddress, suggestion);
        return S7nodaveLongSupport::getPlcDataType(plcAddress, suggestion, *defaultType);
    };

    virtual DBLINK getDeviceAddress() const
    {
        longinRecord *liRec = reinterpret_cast<longinRecord *>(this->record);
        return liRec->inp;
    };

    virtual long writeToRecord(void *buffer, int bufferSize)
    {
        S7nodaveLongSupport::read<longinRecord>(this->myAsynUser, this->record, buffer, bufferSize, this->recordAddress->getPlcDataType());
        return RECORD_STATUS_OK;
    };
};

#endif
