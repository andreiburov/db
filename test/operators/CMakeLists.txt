include_directories(${gtest_SOURCE_DIR}/include)

add_executable(testRegister testRegister.cpp)
target_link_libraries(testRegister database)
target_link_libraries(testRegister gtest gtest_main)

add_executable(testTableScan testTableScan.cpp)
target_link_libraries(testTableScan database)
target_link_libraries(testTableScan gtest gtest_main)