#ifndef S7nodaveWaveformInRecord_h
#define S7nodaveWaveformInRecord_h

#include <waveformRecord.h>
#include <dbFldTypes.h>

#include "S7nodaveInputRecord.h"
#include "s7nodave.h"
#include "s7nodaveAsyn.h"
#include "S7nodaveArraySupport.h"

/**
 * Device support for waveform record, input direction. The waveform record
 * type is special, because it can be used as an input or output record.
 * The two different device supports are implemented by using a different
 * value in the DTYP field (s7nodaveWfIn and s7nodaveWfOut respectively).
 */
class S7nodaveWaveformInRecord : public S7nodaveInputRecord
{
public:
    /**
     * Constructor. The passed record pointer is stored and used by all methods,
     * which need to access record fields.
     */
    S7nodaveWaveformInRecord(dbCommon *record) :
        S7nodaveInputRecord(record, waveformInRecordType)
    {
    };

protected:
    virtual boost::optional<s7nodavePlcDataType> getPlcDataType(S7nodavePlcAddress plcAddress, boost::optional<s7nodavePlcDataType> suggestion)
    {
        waveformRecord *wfRec = reinterpret_cast<waveformRecord *>(this->record);
        return S7nodaveArraySupport::getPlcDataTypeIn(plcAddress, suggestion, wfRec->ftvl);
    };

    virtual DBLINK getDeviceAddress() const
    {
        waveformRecord *wfRec = reinterpret_cast<waveformRecord *>(this->record);
        return wfRec->inp;
    };

    virtual unsigned long int getIoBufferSizeInBits() const
    {
        waveformRecord *wfRec = reinterpret_cast<waveformRecord *>(this->record);
        if (wfRec->ftvl == DBF_STRING) {
            // String is special because each element takes 40 bytes, while
            // the corresponding PLC type is only one byte.
            return MAX_STRING_SIZE * wfRec->nelm * S7nodaveInputRecord::getIoBufferSizeInBits();
        } else {
            return wfRec->nelm * S7nodaveInputRecord::getIoBufferSizeInBits();
        }
    };

    virtual long writeToRecord(void *buffer, int bufferSize)
    {
        return S7nodaveArraySupport::read<waveformRecord>(this->myAsynUser, this->record, buffer, bufferSize, this->recordAddress->getPlcDataType());
    };
};

#endif
