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

constexpr unsigned nearest_odd_number(unsigned x) {
    return x - !(x&1);
}

unsigned divideby2_rounding_up(unsigned x) {
    return x/2 + (x&1);
}

template<typename KEY, typename CMP, unsigned BLOCKSIZE>
class BPlusTree : public Segment {

public:

    struct Node {
        bool leaf;
        unsigned count; // is a count of keys!

        inline bool isLeaf() const {
            return leaf;
        }

        inline unsigned getKeyIndex(const KEY* const keys, const KEY key) const {
            unsigned lo = 0;
            unsigned hi = this->count;

            while (lo < hi) {
                unsigned md = (lo+hi)/2;
                int r = CMP()(keys[md], key);
                if (r == 0) {
                    return md;
                } else if (r < 0) {
                    lo = md + 1;
                } else {
                    hi = md;
                }
            }

            // return where the key should land
            return lo;
        }

        inline virtual bool isFull() const = 0;

        Node(bool leaf) : leaf(leaf), count(0) {}
    };

    struct InnerNode : public Node {
        static const unsigned MAX_COUNT = /*nearest_odd_number(*/ // must be odd, for convenient splitting
                (BLOCKSIZE-sizeof(unsigned)-sizeof(Node)-sizeof(uint64_t))/(sizeof(KEY)+sizeof(uint64_t))/*)*/;
        KEY keys[MAX_COUNT];
        uint64_t refs[MAX_COUNT+1];

        InnerNode() : Node(false) {}

        inline bool isFull() const {
            if (this->count >= MAX_COUNT) {
                return true;
            }
            return false;
        }

        inline unsigned getKeyIndex(const KEY key) const {
            BPlusTree<KEY, CMP, BLOCKSIZE>::Node::getKeyIndex(keys, key);
        }

        inline uint64_t getPage(const KEY key) const {
            return refs[getKeyIndex(key)];
        }

        inline KEY split(InnerNode* node_right) {
            assert(this->count % 2 != 0 && "Split only even number of refs"); // #refs = #keys+1
            unsigned from = this->count/2+1; // excluding middle value
            unsigned len = this->count-from;

            memcpy(node_right->keys, keys+from, len*sizeof(KEY));
            memcpy(node_right->refs, refs+from, (len+1)*sizeof(uint64_t));
            this->count -= len+1; // excluding middle value
            node_right->count += len;

            return keys[this->count];
        }

        inline void init(uint64_t page_id) // should be protected by mutex, root is invalid until right refs is added
        {
            refs[0] = page_id;
        }

        inline void insert(KEY separator, uint64_t page_right) {
            unsigned idx = getKeyIndex(separator); // idx for keys, idx for refs is idx+1
            if (idx >= this->count) // append the values
            {
                keys[idx] = separator;
                refs[idx+1] = page_right;
                this->count++;
                return;
            }

            assert(keys[idx-1] != separator && "Two identical separators! What's inbetween?");

            memmove(keys+idx+1, keys+idx, (this->count-idx)*sizeof(KEY));
            memmove(refs+idx+2, refs+idx+1, (this->count-idx)*sizeof(uint64_t));
            keys[idx] = separator;
            refs[idx+1] = page_right;
            this->count++;
        }

        // for tesing

        inline void insert(KEY key, uint64_t page_id, unsigned idx) {
            keys[idx] = key;
            refs[idx+1] = page_id;
            this->count++;
        }
    };

    struct LeafNode : public Node {
        static const unsigned MAX_COUNT = (BLOCKSIZE-sizeof(unsigned)-sizeof(Node))
                                          /(sizeof(KEY)+sizeof(TID));
        KEY keys[MAX_COUNT];
        TID tids[MAX_COUNT];

        LeafNode() : Node(true) {}

        inline bool isFull() const {
            if (this->count >= MAX_COUNT) {
                return true;
            }
            return false;
        }

        inline unsigned getKeyIndex(const KEY key) const {
            BPlusTree<KEY, CMP, BLOCKSIZE>::Node::getKeyIndex(keys, key);
        }

        inline bool getTID(const KEY key, TID& tid) const {
            unsigned idx = getKeyIndex(key);
            if (keys[idx] == key) {
                tid = tids[idx];
                return true;
            }

            return false;
        }

        inline void insert(KEY key, TID tid) {
            unsigned idx = getKeyIndex(key);
            if (idx >= this->count) // append the values
            {
                keys[idx] = key;
                tids[idx] = tid;
                this->count++;
                return;
            }

            if (keys[idx] == key) { // overwrite
                tids[idx] = tid;
                return;
            }

            memmove(keys+idx+1, keys+idx, (this->count-idx)*sizeof(KEY));
            memmove(tids+idx+1, tids+idx, (this->count-idx)*sizeof(TID));
            keys[idx] = key;
            tids[idx] = tid;
            this->count++;
        }

        inline KEY split(LeafNode* node_right, KEY key, TID tid) {
            unsigned from = this->count/2;
            bool insert_right = false;

            if (CMP()(key, keys[from]) >= 0) {
                insert_right = true;
                from++;
            }

            unsigned len = this->count-from;

            memcpy(node_right->keys, keys+from, len*sizeof(KEY));
            memcpy(node_right->tids, tids+from, len*sizeof(TID));
            this->count -= len;
            node_right->count += len;

            if (insert_right) {
                node_right->insert(key, tid);
            } else {
                insert(key, tid);
            }

            return keys[this->count-1];
        }

        // for testing

        inline void set(KEY key, TID tid, unsigned idx) {
            keys[idx] = key;
            tids[idx] = tid;
            this->count++;
        }
    };

private:

    std::atomic<uint64_t> size_;
    std::atomic<uint64_t> root_;
    std::mutex mutex_;

public:

    BPlusTree(BufferManager& buffer_manager, uint64_t segment_id)
            : Segment(buffer_manager, segment_id), size_(0) , root_(GetFirstPage(segment_id))
    {
        assert(sizeof(InnerNode) <= BLOCKSIZE && "InnerNode larger than BLOCKSIZE");
        assert(sizeof(LeafNode) <= BLOCKSIZE && "LeafNode larger than BLOCKSIZE");

        FrameGuard guard(buffer_manager_, root_, true);
        new(guard.frame.getData()) LeafNode();
        size_++;
    }

    inline uint64_t size() {
        return size_;
    }

    void insert(KEY key, TID value) {
        BufferFrame* frame_prev = nullptr;
        InnerNode* node_prev = nullptr;
        BufferFrame* frame = &buffer_manager_.fixPage(root_, true);
        Node* node = reinterpret_cast<Node*>(frame->getData());
        uint64_t page = root_;

        while (true) {

            if (node->isFull()) {
                std::unique_lock<std::mutex> lock(mutex_); // reserve new page and acquire it
                uint64_t page_right = GetFirstPage(segment_id_) + size_++;
                FrameGuard guard(buffer_manager_, page_right, true);
                lock.unlock();

                if (node_prev == nullptr) { // current node is root
                    lock.lock();
                    uint64_t page_prev = GetFirstPage(segment_id_) + size_++;
                    frame_prev = &buffer_manager_.fixPage(page_prev, true);
                    root_ = page_prev;
                    lock.unlock();
                    node_prev = new(frame_prev->getData()) InnerNode();
                    node_prev->init(page);
                }

                if (node->isLeaf()) {
                    LeafNode* node_right = new(guard.frame.getData()) LeafNode();
                    KEY separator = reinterpret_cast<LeafNode*>(node)->split(node_right, key, value);
                    assert(!node_prev->isFull());
                    node_prev->insert(separator, page_right);
                    break;
                } else { // is inner node
                    InnerNode* node_right = new(guard.frame.getData()) InnerNode();
                    KEY separator = reinterpret_cast<InnerNode*>(node)->split(node_right);
                    assert(!node_prev->isFull());
                    node_prev->insert(separator, page_right);
                }
            } else { // node has space
                if (node->isLeaf()) {
                    reinterpret_cast<LeafNode*>(node)->insert(key, value);
                    break;
                }
            }

            // advance by lock coupling
            node_prev = reinterpret_cast<InnerNode*>(node);
            page = node_prev->getPage(key);
            BufferFrame* temp = frame;
            frame = &buffer_manager_.fixPage(page, true);
            if (frame_prev) buffer_manager_.unfixPage(*frame_prev, true);
            frame_prev = temp;
            node = reinterpret_cast<Node*>(frame->getData());
        }

        buffer_manager_.unfixPage(*frame, true);
        if (frame_prev) buffer_manager_.unfixPage(*frame_prev, true);
    }

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

        BufferFrame* frame = &buffer_manager_.fixPage(root_, exclusive);
        Node* node = reinterpret_cast<Node*>(frame->getData());

        while (!node->isLeaf()) {
            BufferFrame* frame_next =
                    &buffer_manager_.fixPage(reinterpret_cast<InnerNode*>(node)->getPage(key), exclusive);
            node = reinterpret_cast<Node*>(frame_next->getData());
            buffer_manager_.unfixPage(*frame, false);
            frame = frame_next;
        }

        leaf = reinterpret_cast<LeafNode*>(node);
        return *frame;
    }
};

#endif //DB_BPLUSTREE_H
