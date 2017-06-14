#include "gtest/gtest.h"
#include "../../database/dbms.h"
#include "bplustree_helper.h"

/*
 *               16
 *              /  \
 *  1 3 13 15 16   17 18 20 30
 */

TEST(BPlusTree, Lookup) {

    int left[5] = {1,3,13,15,16};
    int right[4] = {17,18,20,30};
    BufferManager bm(3);
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

    n0->refLeft(16, p1, 0);
    n0->refRight(16, p2, 0);

    for (int i = 0; i < sizeof(left)/sizeof(int); ++i) {
        n1->set(left[i], TID(left[i]), i);
    }

    for (int i = 0; i < sizeof(right)/sizeof(int); ++i) {
        n2->set(right[i], TID(right[i]), i);
    }

    for (int i = 0; i < sizeof(left)/sizeof(int); ++i) {
        TID tid;
        btree.lookup(left[i], tid);
        EXPECT_EQ(left[i], tid.page_offset);
    }

    for (int i = 0; i < sizeof(right)/sizeof(int); ++i) {
        TID tid;
        btree.lookup(right[i], tid);
        EXPECT_EQ(right[i], tid.page_offset);
    }
}
