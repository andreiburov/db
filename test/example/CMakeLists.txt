include_directories(${gtest_SOURCE_DIR}/include)

set(example_SOURCE_FILES
    basic_check.cpp
    main.cpp)
add_executable(runExample ${example_SOURCE_FILES})
target_link_libraries(runExample gtest gtest_main)
