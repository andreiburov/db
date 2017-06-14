add_executable(btreetest bplustree_test.cpp)
target_link_libraries(btreetest database)

include_directories(${gtest_SOURCE_DIR}/include)

add_executable(testBPlusTree testBPlusTree.cpp)
target_link_libraries(testBPlusTree database)
target_link_libraries(testBPlusTree gtest gtest_main)

add_executable(testLeafNode testLeafNode.cpp)
target_link_libraries(testLeafNode database)
target_link_libraries(testLeafNode gtest gtest_main)