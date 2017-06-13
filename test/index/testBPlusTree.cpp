#include "gtest/gtest.h"
#include "../../database/dbms.h"

struct IntComparator {
    int operator()(const int& a, const int& b) {
        if (a == b)
            return 0;
        else if (a < b)
            return -1;
        else
            return 1;
    }
};

/*
 *               16
 *              /  \
 *  1 3 13 15 16   17 18 20 30
 */

TEST(BPlusTree, FindLeaf) {
    BufferManager bm(3);
    typedef BPlusTree<int, IntComparator> BPT;
    BPT btree(bm, 2);

    uint64_t p0 = GetFirstPage(2);
    uint64_t p1 = GetFirstPage(2)+1;
    uint64_t p2 = GetFirstPage(2)+2;
    FrameGuard g0(bm, p0, false);
    FrameGuard g1(bm, p1, false);
    FrameGuard g2(bm, p2, false);

    BPT::InnerNode* n0 = new (g0.frame.getData()) BPT::InnerNode();
    BPT::LeafNode* n1 = new (g1.frame.getData()) BPT::LeafNode();
    BPT::LeafNode* n2 = new (g2.frame.getData()) BPT::LeafNode();

    n0->referencePage()
}