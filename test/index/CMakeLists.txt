add_executable(btreetest bplustree_test.cpp)
target_link_libraries(btreetest database)

include_directories(${gtest_SOURCE_DIR}/include)

add_executable(testBPlusTree testBPlusTree.cpp)
target_link_libraries(testBPlusTree database)
target_link_libraries(testBPlusTree gtest gtest_main)