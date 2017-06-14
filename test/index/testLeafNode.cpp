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

typedef BPlusTree<int, IntComparator> BPT;

struct LeafNodeTestData {
    std::set<int> left;
    std::set<int> right;
    int exp_separator;
    int value;

    LeafNodeTestData() : exp_separator(0), value(0) { }

    LeafNodeTestData(std::set<int> left, std::set<int> right, int exp_separator, int value)
            : left(left), right(right), exp_separator(exp_separator), value(value) {}

    LeafNodeTestData(const LeafNodeTestData& t)
            : left(t.left), right(t.right), exp_separator(t.exp_separator), value(t.value) {}
};

class LeafNodeTest : public ::testing::TestWithParam<LeafNodeTestData>
{
protected:

    virtual void SetUp() {
        node_ = new BPT::LeafNode();
        node_right_ = new BPT::LeafNode();
        data_ = GetParam();

        unsigned i = 0;
        for (auto el : data_.left) {
            node_->set(el, TID(el), i++);
        }

        for (auto el : data_.right) {
            node_->set(el, TID(el), i++);
        }
    }

    virtual void TearDown() {
        delete node_;
        delete node_right_;
    }

    BPT::LeafNode* node_;
    BPT::LeafNode* node_right_;
    LeafNodeTestData data_;
};

TEST_P(LeafNodeTest, SplitInsert) {

    int separator = node_->split(node_right_, data_.value, TID(data_.value));
    ASSERT_EQ(data_.exp_separator, separator);

    if (data_.value > *data_.left.rbegin()) {
        data_.right.insert(data_.value);
    } else {
        data_.left.insert(data_.value);
    }

    int i = 0;
    for (auto el : data_.left) {
        EXPECT_EQ(el, node_->keys[i++]);
    }

    i = 0;
    for (auto el : data_.right) {
        EXPECT_EQ(el, node_right_->keys[i++]);
    }
}

std::vector<LeafNodeTestData> leaf_node_test_data =
        {
                { // odd count, insert left
                        {1, 3, 13, 15},
                        {16, 17, 18, 20, 30},
                        15, 4
                },
                { // even count, insert left
                        {1, 3, 13, 15},
                        {16, 17, 18, 20},
                        15, 4
                },
                { // even count, insert center
                        {1, 3, 13, 15},
                        {17, 18, 20, 30},
                        15, 16
                },
                { // odd count, insert right
                        {1, 3, 13, 15},
                        {16, 18, 19, 20, 30},
                        15, 17
                },
                { // even count, insert right
                        {1, 3, 13, 15},
                        {16, 18, 19, 20},
                        15, 17
                }
        };

INSTANTIATE_TEST_CASE_P(LeafNodeTestInstantiation,
        LeafNodeTest,
        ::testing::ValuesIn(leaf_node_test_data));