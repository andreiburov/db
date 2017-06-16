add_executable(btreetest btreetest.cpp)
target_link_libraries(btreetest database)

include_directories(${gtest_SOURCE_DIR}/include)

add_executable(testLookup testLookup.cpp bplustree_helper.h)
target_link_libraries(testLookup database)
target_link_libraries(testLookup gtest gtest_main)

add_executable(testSplitNode testSplitNode.cpp bplustree_helper.h)
target_link_libraries(testSplitNode database)
target_link_libraries(testSplitNode gtest gtest_main)

add_executable(testInsert testInsert.cpp bplustree_helper.h)
target_link_libraries(testInsert database)
target_link_libraries(testInsert gtest gtest_main)