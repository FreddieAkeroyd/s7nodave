#ifndef S7nodavePortDriverReadItem_h
#define S7nodavePortDriverReadItem_h

#include <list>

#include <boost/shared_ptr.hpp>

#include "s7nodaveAsyn.h"

/**
 * Represents an item that should be read (or has been read) from the PLC.
 * This class is intended for use by S7nodavePortDriver and
 * S7nodavePortDriverReadOptimizer only.
 */
class S7nodavePortDriverReadItem
{
public:
    /**
     * Constructor. Constructs a new item that uses an internal buffer using
     * the specified PLC memory address.
     */
    S7nodavePortDriverReadItem(s7nodavePlcArea area, int areaNumber, int startByte, int readBytes);

    /**
     * Constructor. Constructs a new item that is a wrapper around an
     * asynS7nodaveReadItem and its buffer.
     */
    S7nodavePortDriverReadItem(asynS7nodaveReadItem *readItem);

    /**
     * Destructor.
     */
    ~S7nodavePortDriverReadItem();

    /**
     * Returns the PLC area type used.
     */
    inline s7nodavePlcArea getArea() const
    {
        return this->area;
    };

    /**
     * Returns the PLC area number.
     */
    inline int getAreaNumber() const
    {
        return this->areaNumber;
    };

    /**
     * Returns the start byte of the address.
     */
    inline int getStartByte() const
    {
        return this->startByte;
    };

    /**
     * Returns the number of bytes that shall be read.
     */
    inline int getReadBytes() const
    {
        return this->readBytes;
    };

    /**
     * Returns the number of the first byte that is NOT in the read range.
     */
    inline int getEndByte() const
    {
        return this->startByte + this->readBytes;
    };

    /**
     * Returns the number of bytes that have been read.
     */
    inline int getBytesRead() const
    {
        if (usingInternalBuffer) {
            return this->bytesReadInternal;
        } else {
            // Use the bytesRead field in the underlying asynS7nodaveReadItem
            // structure.
            return *(this->bytesReadReference);
        }
    };

    /**
     * Sets the number of bytes that have been read.
     */
    inline void setBytesRead(int bytesRead) {
        if (usingInternalBuffer) {
            this->bytesReadInternal = bytesRead;
        } else {
            // Set the bytesRead field in the underlying asynS7nodaveReadItem
            // structure.
            *this->bytesReadReference = bytesRead;
        }
    };

    /**
     * Returns the buffer that contains the data read from the PLC. If the
     * buffer has been allocated by this item, the returned pointer is only
     * valid as long as this object exists.
     */
    inline void *getBuffer() const
    {
        return buffer;
    };

    /**
     * Adds items that should be included in this item. This only works if the
     * items specify the same memory area as this item. This item is resized
     * to span the whole range of all items contained, if needed. If this item
     * cannot be resized (e.g. because it is backed by an asynS7nodaveReadItem
     * structure), false is returned and the items are not added.
     */
    bool addReadItems(std::list< boost::shared_ptr<S7nodavePortDriverReadItem> > items);

    /**
     * Add an item that should be included in this item. This only works if the
     * item specifies the same memory area as this item. This item is resized
     * to span the whole range of both items, if needed. If this item cannot be
     * resized (e.g. because it is backed by an asynS7nodaveReadItem structure),
     * false is returned and the item is not added.
     */
    bool addReadItem(boost::shared_ptr<S7nodavePortDriverReadItem> item);

    /**
     * Copies the content of this item's buffer to the children's buffers and
     * their children's buffers, etc.
     */
    void dispatchReadResultToChildren();

    /**
     * Checks whether this item includes the whole memory range of the passed
     * item. This method does not check that both items actually refer to the
     * same memory area.
     */
    bool includesNoAreaCheck(const S7nodavePortDriverReadItem& otherItem) const;

    /**
     * Compares the start byte of two items. Can be used to sort a list of items
     * by their start addresses. This method does not compare the memory area.
     */
    static bool compareStartByte(const S7nodavePortDriverReadItem& first, const S7nodavePortDriverReadItem& second);

    /**
     * Wrapper around compareStartByte that can be used when the two items to be
     * compared are both wrapped in shared_ptr containers.
     */
    static bool pointerCompareStartByte(const boost::shared_ptr<S7nodavePortDriverReadItem>& first, const boost::shared_ptr<S7nodavePortDriverReadItem>& second);

private:
    /*
     * Private copy constructor. Objects of this class should never be copied.
     */
    S7nodavePortDriverReadItem(const S7nodavePortDriverReadItem &);

    /**
     * Private assignment operator. Objects of this class should never be
     * copied.
     */
    S7nodavePortDriverReadItem& operator=(const S7nodavePortDriverReadItem &);

    /**
     * Items included in this item.
     */

    std::list< boost::shared_ptr<S7nodavePortDriverReadItem> > includedReadItems;

    /**
     * PLC memory area type.
     */
    s7nodavePlcArea area;

    /**
     * PLC memory area number.
     */
    int areaNumber;

    /**
     * PLC address start byte.
     */
    int startByte;

    /**
     * Number of bytes that shall be read from the PLC.
     */
    int readBytes;

    /**
     * Pointer to the bytesRead field of the underlying asynS7nodaveReadItem
     * structure or NULL, if this object has been constructed without using
     * such a structure.
     */
    int *bytesReadReference;

    /**
     * Number of bytes read. Only used if this object has been constructed
     * without using an asynS7nodaveReadItem structure.
     */
    int bytesReadInternal;

    /**
     * Buffer for storing read data. Might either be allocated internally or
     * a reference to the buffer of an asynS7nodaveReadItem structure.
     */
    void *buffer;

    /**
     * True if buffer has been allocated internally.
     */
    bool usingInternalBuffer;
};

#endif
