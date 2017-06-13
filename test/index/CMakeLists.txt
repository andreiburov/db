add_executable(btreetest bplustree_test.cpp)
target_link_libraries(btreetest database)

include_directories(${gtest_SOURCE_DIR}/include)

#add_executable(testSPSegment testSPSegment.cpp)
#target_link_libraries(testSPSegment database)
#target_link_libraries(testSPSegment gtest gtest_main)