#include "gtest/gtest.h"
#include "../../subscripts/arithmetic/ArithmeticModule.h"

ArithmeticModule amc;

TEST(ArithmeticModule, Addition)
{
    Node root(Node::Type::ADDITION);
    root.left = std::unique_ptr<Node>(new Node("v0"));
    root.right = std::unique_ptr<Node>(new Node(23));

    Function* add = amc.addFunction("Addition", root);

    std::vector<int64_t> params;
    params.push_back(1);

    int64_t ret;

    ASSERT_TRUE(amc.runFunction(add, params, ret)) << "error when running the function";
    ASSERT_EQ(params[0]+23, ret);
}

// (v0 + v1) * (v2 − v3))
// (1 + 2) * (5 − 2) = 9)
TEST(ArithmeticModule, Complex)
{
    Node root(Node::Type::MULTIPLICATION);
    root.left = std::unique_ptr<Node>(new Node(Node::Type::ADDITION));
    root.right = std::unique_ptr<Node>(new Node(Node::Type::SUBTRACTION));

    Node* left = root.left.get();
    Node* right = root.right.get();

    left->left = std::unique_ptr<Node>(new Node("v0"));
    left->right = std::unique_ptr<Node>(new Node("v1"));
    right->left = std::unique_ptr<Node>(new Node("v2"));
    right->right = std::unique_ptr<Node>(new Node("v3"));

    Function* add = amc.addFunction("Complex", root);

    std::vector<int64_t> params;
    params.push_back(1);
    params.push_back(2);
    params.push_back(5);
    params.push_back(2);

    int64_t ret;

    ASSERT_TRUE(amc.runFunction(add, params, ret)) << "error when running the function";
    ASSERT_EQ(9, ret);
}