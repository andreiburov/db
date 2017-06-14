#include "gtest/gtest.h"
#include "../../database/dbms.h"
#include "bplustree_helper.h"

std::vector<int> values = {16, 3, 17, 5, 10, 19, 15, 14, 13};

TEST(BPlusTree, Insert) {
    const unsigned blocksize = 64;
    typedef BPlusTree<int, IntComparator, blocksize> B;

    if (B::InnerNode::MAX_COUNT != 3)
        FAIL() << "Wrong MAX_COUNT for InnerNode" << std::endl;
    if (B::LeafNode::MAX_COUNT != 3)
        FAIL() << "Wrong MAX_COUNT for LeafNode" << std::endl;

    BufferManager bm(100);
    B tree(bm, 2);

    for (auto v : values) {
        tree.insert(v, TID(v));
    }

    ASSERT_EQ(1,1);
}



