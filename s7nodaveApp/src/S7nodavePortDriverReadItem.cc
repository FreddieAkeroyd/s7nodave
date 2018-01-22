#include <string.h>

#include "S7nodavePortDriverReadItem.h"

using boost::shared_ptr;
using std::list;

typedef S7nodavePortDriverReadItem ReadItem;
typedef shared_ptr<S7nodavePortDriverReadItem> ReadItemPtr;
typedef list<ReadItemPtr> ReadItemList;

ReadItem::S7nodavePortDriverReadItem(s7nodavePlcArea area, int areaNumber, int startByte, int readBytes) :
    area(area),
    areaNumber(areaNumber),
    startByte(startByte),
    readBytes(readBytes),
    bytesReadReference(NULL),
    bytesReadInternal(0),
    buffer(NULL),
    // We use the internal buffer, because none was passed.
    usingInternalBuffer(true)
{
    // Reserve space for the buffer
    buffer = pasynManager->memMalloc(this->readBytes);
};

ReadItem::S7nodavePortDriverReadItem(asynS7nodaveReadItem *readItem) :
    area(readItem->area),
    areaNumber(readItem->areaNumber),
    startByte(readItem->startByte),
    readBytes(readItem->readBytes),
    bytesReadReference(&readItem->bytesRead),
    bytesReadInternal(0),
    buffer(readItem->buffer),
    usingInternalBuffer(false)
{
};

ReadItem::~S7nodavePortDriverReadItem()
{
    if (usingInternalBuffer && buffer != NULL) {
        pasynManager->memFree(this->buffer, this->readBytes);
        this->buffer = NULL;
    }
}

bool ReadItem::addReadItems(std::list< boost::shared_ptr<S7nodavePortDriverReadItem> > items)
{
    int newStartByte = this->startByte;
    int newEndByte = this->getEndByte();
    for (std::list< boost::shared_ptr<S7nodavePortDriverReadItem> >::iterator i = items.begin(); i != items.end(); i++) {
        boost::shared_ptr<S7nodavePortDriverReadItem> item = *i;
        newStartByte = std::min(newStartByte, item->getStartByte());
        newEndByte = std::max(newEndByte, item->getEndByte());
    }
    int newLength = newEndByte - newStartByte;
    if (newStartByte != this->startByte || newLength != this->readBytes) {
        if (!usingInternalBuffer || bytesReadInternal != 0) {
            return false;
        }
        // Resize buffer
        void *newBuffer = pasynManager->memMalloc(newLength);
        if (newBuffer == NULL) {
            return false;
        }
        pasynManager->memFree(this->buffer, this->readBytes);
        this->buffer = newBuffer;
        this->startByte = newStartByte;
        this->readBytes = newLength;
    }
    // Insert all items into list
    this->includedReadItems.splice(this->includedReadItems.end(), items);
    return true;
}

bool ReadItem::addReadItem(boost::shared_ptr<S7nodavePortDriverReadItem> item)
{
    if (!item) {
        return false;
    }
    if (this->area != item->area || this->areaNumber != this->areaNumber) {
        return false;
    }
    int newStartByte = std::min(this->startByte, item->startByte);
    int newEndByte = std::max(this->getEndByte(), item->getEndByte());
    int newLength = newEndByte - newStartByte;
    if (newStartByte != this->startByte || newLength != this->readBytes) {
        if (!usingInternalBuffer || bytesReadInternal != 0) {
            return false;
        }
        // Resize buffer
        void *newBuffer = pasynManager->memMalloc(newLength);
        if (newBuffer == NULL) {
            return false;
        }
        pasynManager->memFree(this->buffer, this->readBytes);
        this->buffer = newBuffer;
        this->startByte = newStartByte;
        this->readBytes = newLength;
    }
    this->includedReadItems.push_back(item);
    return true;
}

void ReadItem::dispatchReadResultToChildren()
{
    for (ReadItemList::iterator it = this->includedReadItems.begin(); it != this->includedReadItems.end(); it++) {
        ReadItemPtr item2 = *it;
        // Copy buffer to target item
        void *buffer1 = this->buffer;
        void *buffer2 = item2->getBuffer();
        int bytesRead = this->readBytes;
        if (bytesRead != 0 && buffer1 != NULL && buffer2 != NULL) {
            // There might be an offset between the buffer in item1 and the
            // start of the range item2 is interested in.
            int offset = (item2->getStartByte() - this->startByte);
            // If an error occurred, there might be less bytes available in
            // buffer1 than asked for by buffer2. In this case, we only read
            // the available bytes.
            bytesRead = std::min(bytesRead - offset, item2->getReadBytes());
            // We have to set the number of bytes read in item2.
            item2->setBytesRead(bytesRead);
            // memcpy does not accept an offset, so we have to change the
            // pointer accordingly.
            void *source = static_cast<void *>(static_cast<char *>(buffer1) + offset);
            memcpy(buffer2, source, bytesRead);
        } else {
            item2->setBytesRead(0);
        }
        // Process items included by process item
        item2->dispatchReadResultToChildren();
    }
}

bool ReadItem::includesNoAreaCheck(const ReadItem& other) const
{
    if (this->startByte > other.startByte) {
        return false;
    }
    if (this->getEndByte() < other.getEndByte()) {
        return false;
    }
    return true;
}

bool ReadItem::compareStartByte(const ReadItem& first, const ReadItem& second)
{
    if (first.startByte < second.startByte) {
        return true;
    } else {
        return false;
    }
}

bool ReadItem::pointerCompareStartByte(const boost::shared_ptr<S7nodavePortDriverReadItem>& first, const boost::shared_ptr<S7nodavePortDriverReadItem>& second)
{
    if (!first && second) {
        return true;
    } else if ((first && !second) || (!first && !second)) {
        return false;
    } else {
        return compareStartByte(*first, *second);
    }
}
