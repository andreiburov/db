include_directories(${gtest_SOURCE_DIR}/include)

add_executable(runExample basic_check.cpp)
target_link_libraries(runExample gtest gtest_main)