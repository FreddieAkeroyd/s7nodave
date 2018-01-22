#ifndef S7nodaveMbboDirectRecord_h
#define S7nodaveMbboDirectRecord_h

#include <mbboDirectRecord.h>

#include "S7nodaveOutputRecord.h"
#include "s7nodave.h"
#include "s7nodaveAsyn.h"
#include "S7nodaveMultiBinarySupport.h"

/**
 * Device support for mbboDirect record.
 */
class S7nodaveMbboDirectRecord : public S7nodaveOutputRecord
{
public:
    /**
     * Constructor. The passed record pointer is stored and used by all methods,
     * which need to access record fields.
     */
    S7nodaveMbboDirectRecord(dbCommon *record) :
        S7nodaveOutputRecord(record, mbboDirectRecordType)
    {

    };

    virtual long initRecord() {
        long status = S7nodaveOutputRecord::initRecord();
        if (status != RECORD_STATUS_OK) {
            return status;
        }

        mbboDirectRecord *r = reinterpret_cast<mbboDirectRecord *>(this->record);
        epicsUInt32 rval = r->rval;
        if (r->shft > 0) {
            rval >>= r->shft;
        }
        if (r->mask != 0) {
            rval &= r->mask;
        }
        epicsUInt8 *bits[] = {&r->b0, &r->b1, &r->b2, &r->b3, &r->b4, &r->b5, &r->b6, &r->b7, &r->b8, &r->b9, &r->ba, &r->bb, &r->bc, &r->bd, &r->be, &r->bf};
        for (int i = 0; i < 16; i++) {
            if ((rval & (0x01 << i)) != 0) {
                *bits[i] = 1;
            } else {
                *bits[i] = 0;
            }
        }

        return status;
    }

protected:
    virtual long interceptInitRecordBeforeValueInit()
    {
        S7nodaveMultiBinarySupport::initMask<mbboDirectRecord>(this->record, this->recordAddress->getPlcAddress().getDataSize());
        return RECORD_STATUS_OK;
    };

    virtual boost::optional<s7nodavePlcDataType> getPlcDataType(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> suggestion)
    {
        boost::optional<s7nodavePlcDataType> defaultType = S7nodaveOutputRecord::getPlcDataType(plcAddress, suggestion);
        return S7nodaveMultiBinarySupport::getPlcDataType(plcAddress, suggestion, *defaultType);
    };

    virtual DBLINK getDeviceAddress() const
    {
        mbboDirectRecord *mbboDRec = reinterpret_cast<mbboDirectRecord *>(this->record);
        return mbboDRec->out;
    };

    virtual void readFromRecord(void *buffer, int bufferSize) const
    {
        S7nodaveMultiBinarySupport::write<mbboDirectRecord>(this->myAsynUser, this->record, buffer, bufferSize, this->recordAddress->getPlcDataType());
    };

    virtual long writeToRecord(void *buffer, int bufferSize)
    {
        S7nodaveMultiBinarySupport::read<mbboDirectRecord>(this->myAsynUser, this->record, buffer, bufferSize, this->recordAddress->getPlcDataType());
        return RECORD_STATUS_OK;
    };
};

#endif
