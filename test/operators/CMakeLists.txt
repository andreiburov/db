include_directories(${gtest_SOURCE_DIR}/include)

add_executable(testRegister testRegister.cpp)
target_link_libraries(testRegister database)
target_link_libraries(testRegister gtest gtest_main)

add_executable(testTableScan testTableScan.cpp)
target_link_libraries(testTableScan database)
target_link_libraries(testTableScan gtest gtest_main)

add_executable(testHashInnerJoin testHashInnerJoin.cpp)
target_link_libraries(testHashInnerJoin database)
target_link_libraries(testHashInnerJoin gtest gtest_main)

add_executable(testSelection testSelection.cpp)
target_link_libraries(testSelection database)
target_link_libraries(testSelection gtest gtest_main)

add_executable(testProjection testProjection.cpp)
target_link_libraries(testProjection database)
target_link_libraries(testProjection gtest gtest_main)

add_executable(testPrint testPrint.cpp)
target_link_libraries(testPrint database)
target_link_libraries(testPrint gtest gtest_main)