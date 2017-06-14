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

constexpr unsigned nearest_odd_number(unsigned x){
    return x - ~(x&1);
}

template<typename KEY, typename CMP>
class BPlusTree : public Segment {

public:

    struct Node {
        bool leaf;
        unsigned count;

        inline bool isLeaf() const {
            return leaf;
        }

        inline unsigned getKeyIndex(const KEY* const keys, const KEY key) const {
            unsigned lo = 0;
            unsigned hi = this->count - 1;

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
        static const unsigned MAX_COUNT = nearest_odd_number( // must be odd, for convenient splitting
                (PAGESIZE-sizeof(unsigned)-sizeof(Node)-sizeof(uint64_t))/(sizeof(KEY)+sizeof(uint64_t)));
        KEY keys[MAX_COUNT];
        uint64_t refs[MAX_COUNT+1];

        InnerNode() : Node(false) {}
        InnerNode(uint64_t left_ref) : Node(false) { refs[0] = left_ref; this->count++; }

        inline virtual bool isFull() const { // virtual for testing with sizes other than PAGESIZE
            if (this->count >= MAX_COUNT+1) { // count references
                return true;
            }
            return false;
        }

        inline unsigned getKeyIndex(const KEY key) const {
            BPlusTree<KEY, CMP>::Node::getKeyIndex(keys, key);
        }

        inline uint64_t getPage(const KEY key) const {
            return refs[getKeyIndex(key)];
        }

        inline KEY split(InnerNode* node_right) {
            assert(this->count % 2 == 0 && "Split only even number of refs");
            unsigned from = this->count/2;
            unsigned len = this->count-from;

            memcpy(node_right->keys, keys+from, (len-1)*sizeof(KEY));
            memcpy(node_right->refs, refs+from, len*sizeof(uint64_t));
            this->count -= len; // excluding middle value
            node_right->count += len;

            return keys[this->count-1];
        }

        inline void insert(KEY separator, uint64_t page_right) {
            unsigned idx = getKeyIndex(separator);
            assert(keys[idx] != separator && "Attempt to insert the value, that is already stored");

            memmove(keys+idx+1, keys+idx, (this->count-idx)*sizeof(KEY));
            memmove(refs+idx+2, refs+idx+1, (this->count-idx-1)*sizeof(uint64_t));
            keys[idx] = separator;
            refs[idx+1] = page_right;
            this->count++;
        }

        // for tesing

        inline void refLeft(KEY key, uint64_t page_id, unsigned idx) {
            keys[idx] = key;
            refs[idx] = page_id;
            this->count++;
        }

        inline void refRight(KEY key, uint64_t page_id, unsigned idx) {
            keys[idx] = key;
            refs[idx+1] = page_id;
            this->count++;
        }
    };

    struct LeafNode : public Node {
        static const unsigned MAX_COUNT = (PAGESIZE-sizeof(unsigned)-sizeof(Node))
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
            BPlusTree<KEY, CMP>::Node::getKeyIndex(keys, key);
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
            if (keys[idx] == key) {
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
            unsigned len = this->count-from;

            memcpy(node_right->keys, keys+from, len*sizeof(KEY));
            memcpy(node_right->tids, tids+from, len*sizeof(TID));
            this->count -= len;
            node_right->count += len;

            if (CMP()(key, keys[this->count-1]) > 0) {
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
        assert(sizeof(InnerNode) <= PAGESIZE && "InnerNode larger than PAGESIZE");
        assert(sizeof(LeafNode) <= PAGESIZE && "LeafNode larger than PAGESIZE");

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
            bool dirty = false;

            if (node->isFull()) {
                std::unique_lock<std::mutex> lock(mutex_); // reserve new page and acquire it
                uint64_t page_right = ++size_;
                FrameGuard guard(buffer_manager_, page_right, true);
                lock.unlock();

                if (node->isLeaf()) {
                    LeafNode* node_right = new(guard.frame.getData()) LeafNode();
                    KEY separator = reinterpret_cast<LeafNode*>(node)->split(node_right, key, value);
                    assert(!node_prev->isFull());
                    node_prev->insert(separator, page_right);
                    break;
                } else { // is inner node
                    InnerNode* node_right = new(guard.frame.getData()) InnerNode();
                    KEY separator = reinterpret_cast<InnerNode*>(node)->split(node_right);
                    if (node_prev == nullptr) // node is root
                    {
                        lock.lock();
                        uint64_t page_root = ++size_;
                        FrameGuard guard_root(buffer_manager_, page_root, true);
                        lock.unlock();
                        InnerNode* node_root = new(guard_root.frame.getData()) InnerNode(page);
                        node_root->insert(separator, page_right);
                    } else { // node in not root
                        assert(!node_prev->isFull());
                        node_prev->insert(separator, page_right);
                    }
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
            frame = &buffer_manager_.fixPage(page, true);
            buffer_manager_.unfixPage(*frame_prev, dirty);
            frame_prev = frame;
            node = reinterpret_cast<Node*>(frame->getData());
        }
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
