add_executable(slottedtest slottedtest.cpp)
target_link_libraries(slottedtest database)

include_directories(${gtest_SOURCE_DIR}/include)

#add_executable(testPageIO testPageIO.cpp)
#target_link_libraries(testPageIO database)
#target_link_libraries(testPageIO gtest gtest_main)