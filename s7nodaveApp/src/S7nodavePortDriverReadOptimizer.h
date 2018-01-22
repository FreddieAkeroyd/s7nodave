#ifndef S7nodavePortDriverReadOptimizer_h
#define S7nodavePortDriverReadOptimizer_h

#include <list>

#include <boost/shared_ptr.hpp>

#include "S7nodavePortDriverReadItem.h"

/**
 * Optimized a set of read items. This optimization is performed by aggregating
 * read items that refer to the same or close-by memory ranges into a single
 * item, thus reducing the number of items and avoiding memory ranges to be
 * transferred twice.
 * This class is intended for use by S7nodavePortDriver only.
 */
class S7nodavePortDriverReadOptimizer
{
public:
    typedef S7nodavePortDriverReadItem ReadItem;
    typedef boost::shared_ptr<ReadItem> ReadItemPtr;
    typedef std::list<ReadItemPtr> ReadItemList;

    /**
     * Creates an optimized list of read items from the passed chained read item
     * structure. The maximum payload size passed is the maximum payload size
     * PDU size without the 14 byte command header. For each item requested,
     * another 4 bytes have to be allocated for the item header.
     */
    static ReadItemList createOptimizedList(asynS7nodaveReadItem *firstItem, int maxPayloadSize);

    /**
     * Calls the dispatchReadResultsToChildren of each read item in the list
     * and subsequently removes the item from the list.
     */
    static void dispatchResults(ReadItemList& readItemList);
private:
    /**
     * Private constructor. The public methods of this class are all static, so
     * there is no need to create an instance.
     */
    S7nodavePortDriverReadOptimizer() {};

    /**
     * Called by createOptimizedList to optimize the number of items within
     * a single memory area.
     */
    static void optimizeWithinArea(ReadItemList& readItemList, int maxPayloadSize);
};

#endif
