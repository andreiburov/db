#include "gtest/gtest.h"
#include "../../database/dbms.h"
#include "bplustree_helper.h"

template<typename SIZE>
class LeafNodeFake : public BPT::LeafNode {
public:
    virtual bool isFull() const final {
        if (this->count >= SIZE()()) {
            return true;
        }
        return false;
    }
};

template<typename SIZE>
class InnerNodeFake : public BPT::InnerNode {
public:
    virtual bool isFull() const final {
        if (this->count >= SIZE()()) {
            return true;
        }
        return false;
    }
};

struct LeafNodeSize {
    inline unsigned operator()() {
        return 4;
    }
};

TEST(BPlusTree, Insert) {

}




