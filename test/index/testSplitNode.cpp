#include "gtest/gtest.h"
#include "../../database/dbms.h"
#include "bplustree_helper.h"

struct NodeTestData {
    std::set<int> left;
    std::set<int> right;
    int exp_separator;
    int value;

    NodeTestData() : exp_separator(0), value(0) { }

    NodeTestData(std::set<int> left, std::set<int> right, int exp_separator, int value)
            : left(left), right(right), exp_separator(exp_separator), value(value) {}

    NodeTestData(const NodeTestData& t)
            : left(t.left), right(t.right), exp_separator(t.exp_separator), value(t.value) {}
};

class NodeTest : public ::testing::TestWithParam<NodeTestData> {
protected:

    virtual void SetUp() {
        data_ = GetParam();
    }

    NodeTestData data_;
};

// LeafTest

class LeafTest : public NodeTest {
protected:

    virtual void SetUp() {

        NodeTest::SetUp();

        node_ = new BPT::LeafNode();
        node_right_ = new BPT::LeafNode();

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
};

TEST_P(LeafTest, Split) {

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
    ASSERT_EQ(i, node_->count);

    i = 0;
    for (auto el : data_.right) {
        EXPECT_EQ(el, node_right_->keys[i++]);
    }
    ASSERT_EQ(i, node_right_->count);
}

std::vector<NodeTestData> leaf_node_test_data =
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

INSTANTIATE_TEST_CASE_P(LeafTestInstantiation,
        LeafTest,
        ::testing::ValuesIn(leaf_node_test_data));


class InnerTest : public NodeTest {
protected:

    virtual void SetUp() {

        NodeTest::SetUp();
        node_ = new BPT::InnerNode();
        node_right_ = new BPT::InnerNode();

        unsigned i = 0;
        node_->refLeft(*data_.left.begin(), -1, i);

        for (auto el : data_.left) {
            node_->refRight(el, el, i++);
        }

        for (auto el : data_.right) {
            node_->refRight(el, el, i++);
        }
    }

    virtual void TearDown() {
        delete node_;
        delete node_right_;
    }

    BPT::InnerNode* node_;
    BPT::InnerNode* node_right_;
};

TEST_P(InnerTest, Split) {

    if (node_->count % 2 != 0) // number of references not even
    {
        EXPECT_EXIT(node_->split(node_right_), ::testing::KilledBySignal(SIGABRT), "Split only even number of refs");
    } else {
        int separator = node_->split(node_right_);

        ASSERT_EQ(data_.exp_separator, separator);

        if (data_.left.size() > data_.right.size()) {
            data_.left.erase(std::prev(data_.left.end()));
        } else {
            data_.right.erase(data_.right.begin());
        }

        int i = 0;
        ASSERT_EQ(-1, node_->refs[i]);
        for (auto el : data_.left) {
            EXPECT_EQ(el, node_->keys[i++]);
            EXPECT_EQ(el, node_->refs[i]);
        }
        ASSERT_EQ(i+1, node_->count);

        i = 0;
        ASSERT_EQ(data_.exp_separator, node_right_->refs[i]);
        for (auto el : data_.right) {
            EXPECT_EQ(el, node_right_->keys[i++]);
            EXPECT_EQ(el, node_right_->refs[i]);
        }
        ASSERT_EQ(i+1, node_right_->count);
    }

}

std::vector<NodeTestData> inner_node_test_data =
        {
                { // even number of keys should produce assertion error in split
                        {1, 3, 13, 14},
                        {16, 17, 18, 20},
                        15, -1
                },
                { // separator taken from the left side
                        {1, 3, 13, 14, 15},
                        {16, 17, 18, 20},
                        15, -1
                },
                { // separator taken from the right side
                        {1, 3, 13, 14},
                        {16, 17, 18, 20, 44},
                        16, -1
                }
        };

INSTANTIATE_TEST_CASE_P(InnerTestInstantiation,
                        InnerTest,
                        ::testing::ValuesIn(inner_node_test_data));