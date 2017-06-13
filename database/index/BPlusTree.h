#ifndef DB_BPLUSTREE_H
#define DB_BPLUSTREE_H

#include <atomic>
#include <cassert>
#include "../slotted_pages/Segment.h"
#include "../slotted_pages/TID.h"
#include "../dbms.h"

/* B+ Tree index segment
 *
 * Each node is a datablock (usually page size)
 *
 * Must support the following reentrant operations
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

template<typename KEY, typename CMP>
class BPlusTree : public Segment {

public:

    struct Node {
        bool leaf;
        unsigned count;

        inline bool isLeaf() {
            return leaf;
        }

        inline virtual unsigned getKeyIndex(KEY key) = 0;

        Node(bool leaf) : leaf(leaf), count(0) {}
    };

    struct InnerNode : public Node {
        static const uint64_t MAX_COUNT = (PAGESIZE-sizeof(uint64_t)-sizeof(Node))
                /(sizeof(KEY)+sizeof(uint64_t))-1;
        KEY keys[MAX_COUNT];
        uint64_t refs[MAX_COUNT+1];

        InnerNode() : Node(false) {}

        inline unsigned getKeyIndex(KEY key) {
            unsigned lo = 0;
            unsigned hi = count - 1;

            while (lo < hi) {
                unsigned md = (lo+hi)/2;
                int r = CMP(keys[md], key);
                if (r == 0) {
                    return md;
                } else if (r < 0) {
                    lo = md;
                } else {
                    hi = md;
                }
            }

            assert(lo == hi);
            return lo;
        }

        inline uint64_t getPage(KEY key) {
            return refs[getKeyIndex(key)];
        }

        inline void referenceLeft(KEY key, uint64_t page_id, unsigned idx) {
            keys[idx] = key;
            refs[idx] = page_id;
            count++;
        }

        inline void referenceRight(KEY key, uint64_t page_id, unsigned idx) {
            keys[idx] = key;
            refs[idx+1] = page_id;
            count++;
        }
    };

    struct LeafNode : public Node {
        static const uint64_t MAX_COUNT = (PAGESIZE-sizeof(uint64_t)-sizeof(Node))
                                          /(sizeof(KEY)+sizeof(TID));
        KEY keys[MAX_COUNT];
        TID tids[MAX_COUNT];

        LeafNode() : Node(true) {}

        inline unsigned getKeyIndex(KEY key) {
            unsigned lo = 0;
            unsigned hi = count - 1;

            while (lo < hi) {
                unsigned md = (lo+hi)/2;
                int r = CMP(keys[md], key);
                if (r == 0) {
                    return md;
                } else if (r < 0) {
                    lo = md;
                } else {
                    hi = md;
                }
            }

            assert(lo == hi);
            return lo;
        }

        inline bool getTID(KEY key, TID& tid) {
            unsigned idx = getKeyIndex(key);
            if (keys[idx] == key) {
                tid = tids[idx];
                return true;
            }

            return false;
        }
    };

private:

    std::atomic<uint64_t> size_;
    std::atomic<uint64_t> root_;

public:

    BPlusTree(BufferManager& buffer_manager, uint64_t segment_id)
            : Segment(buffer_manager, segment_id), size_(0) , root_(GetFirstPage(segment_id))
    {
        assert(sizeof(InnerNode) <= PAGESIZE);
        assert(sizeof(LeafNode) <= PAGESIZE);

        FrameGuard guard(buffer_manager_, root_, true);
        new(guard.frame.getData()) LeafNode();
        size_++;
    }

    inline uint64_t size() {
        return size_;
    }

    void insert(KEY key, TID value);

    void erase(KEY key);

    bool lookup(KEY key, TID& tid) {
        LeafNode* leaf = nullptr;
        BufferFrame& frame = findLeaf(key, leaf, false);
        assert(leaf->isLeaf());
        bool found = leaf->getTID(key, tid);
        buffer_manager_.unfixPage(frame, false);
        return found;
    }

    // private: exposed for tests

    // caller must release node frame
    BufferFrame& findLeaf(const KEY key, LeafNode* &leaf, const bool exclusive) const {

        BufferFrame& frame = buffer_manager_.fixPage(root_, false);
        Node* node = reinterpret_cast<Node*>(frame.getData());

        while (!node->isLeaf()) {
            BufferFrame& frame_child =
                    buffer_manager_.fixPage(reinterpret_cast<InnerNode*>(node)->getPage(key), false);
            node = reinterpret_cast<Node*>(frame_child.getData());
            buffer_manager_.unfixPage(frame, false);
            frame = frame_child;
        }

        leaf = reinterpret_cast<LeafNode*>(node);
        return frame;
    }
};

#endif //DB_BPLUSTREE_H
