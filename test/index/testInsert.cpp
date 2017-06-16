#include "gtest/gtest.h"
#include "../../database/dbms.h"
#include "bplustree_helper.h"

/*              14(n0)
 *            /       \
 *           /         \
 *         5|12(n1)      16(n2)
 *      /   |   \          |  \
 * (n3)/(n4)|(n5)\     (n6)|   \(n7)
 *  3|5  10|12 13|14   15|16 17|19
 */

typedef std::vector<int> Node;

template <typename TREE , typename NODE>
NODE* GetNode(BufferManager& bm, uint64_t page) {
    BufferFrame& f = bm.fixPage(page, true);
    NODE* e = reinterpret_cast<NODE*>(f.getData());
    bm.unfixPage(f, false);
    return e;
};

template <typename NODE>
void CheckNode(Node expected, NODE* n) {
    unsigned i = 0;
    for (auto el : expected) {
        EXPECT_EQ(el, n->keys[i++]);
    }
    EXPECT_EQ(expected.size(), i);
}

TEST(BPlusTree, Insert) {
    Node values = {16, 3, 17, 5, 10, 19, 15, 14, 13, 12};
    Node e0 = {14};
    Node e1 = {5,12};
    Node e2 = {16};
    Node e3 = {3,5};
    Node e4 = {10,12};
    Node e5 = {13,14};
    Node e6 = {15,16};
    Node e7 = {17,19};

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

    B::InnerNode* n0 = GetNode<B, B::InnerNode>(bm, tree.root_);
    B::InnerNode* n1 = GetNode<B, B::InnerNode>(bm, n0->refs[0]);
    B::InnerNode* n2 = GetNode<B, B::InnerNode>(bm, n0->refs[1]);
    B::LeafNode* n3 = GetNode<B, B::LeafNode>(bm, n1->refs[0]);
    B::LeafNode* n4 = GetNode<B, B::LeafNode>(bm, n1->refs[1]);
    B::LeafNode* n5 = GetNode<B, B::LeafNode>(bm, n1->refs[2]);
    B::LeafNode* n6 = GetNode<B, B::LeafNode>(bm, n2->refs[0]);
    B::LeafNode* n7 = GetNode<B, B::LeafNode>(bm, n2->refs[1]);

    CheckNode<B::InnerNode>(e0, n0);
    CheckNode<B::InnerNode>(e1, n1);
    CheckNode<B::InnerNode>(e2, n2);
    CheckNode<B::LeafNode>(e3, n3);
    CheckNode<B::LeafNode>(e4, n4);
    CheckNode<B::LeafNode>(e5, n5);
    CheckNode<B::LeafNode>(e6, n6);
    CheckNode<B::LeafNode>(e7, n7);
}

TEST(BPlusTree, InsertSequence) {
    const unsigned blocksize = 64;
    typedef BPlusTree<int, IntComparator, blocksize> B;

    BufferManager bm(100);
    B tree(bm, 2);

    uint64_t n = 12;
    for (uint64_t i=0; i<n; ++i)
        tree.insert(i,static_cast<TID>(i*i));
    assert(tree.size()==n);

    for (uint64_t i=0; i<n; ++i) {
        TID tid;
        assert(tree.lookup(i,tid));
        assert(tid==i*i);
    }
}

TEST(BPlusTree, InsertReverse) {
    const unsigned blocksize = 64;
    typedef BPlusTree<int, IntComparator, blocksize> B;

    BufferManager bm(100);
    B tree(bm, 2);

    uint64_t n = 12;
    for (uint64_t i=n; i>=1; --i)
        tree.insert(i,static_cast<TID>(i*i));
    assert(tree.size()==n);

    for (uint64_t i=n; i>=1; --i) {
        TID tid;
        assert(tree.lookup(i,tid));
        assert(tid==i*i);
    }
}