add_executable(slottedtest slottedtest.cpp)
target_link_libraries(slottedtest database)

include_directories(${gtest_SOURCE_DIR}/include)

add_executable(testSPSegment testSPSegment.cpp)
target_link_libraries(testSPSegment database)
target_link_libraries(testSPSegment gtest gtest_main)

add_executable(testSlot testSlot.cpp)
target_link_libraries(testSlot database)
target_link_libraries(testSlot gtest gtest_main)