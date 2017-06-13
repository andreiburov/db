#ifndef DB_BPLUSTREE_H
#define DB_BPLUSTREE_H

#include <atomic>

/* B+ Tree index segment
 *
 * must support the following reentrant operations
 *
 * - insert
 *     Inserts a new key/TID pair into the tree.
 *
 * - erase
 *     Deletes a specified key. You may simplify the logic by accepting under full pages.
 *
 * - lookup
 *     Returns a TID or indicates that the key was not found
 */

template<typename T, typename CMP>
class BPlusTree : public Segment {

    std::atomic<uint64_t> size_;

public:

    BPlusTree(BufferManager& buffer_manager, uint64_t segment_id)
            : Segment(buffer_manager, segment_id), size_(0) {}

    inline uint64_t size() {
        return size_.load();
    }

    void insert(T key, TID value);

    void erase(T key);

    bool lookup(T key, TID& value);
};

#endif //DB_BPLUSTREE_H
