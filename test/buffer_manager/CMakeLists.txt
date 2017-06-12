add_executable(buffertest buffertest.cpp)
target_link_libraries(buffertest database)

include_directories(${gtest_SOURCE_DIR}/include)

add_executable(testPageIO testPageIO.cpp)
target_link_libraries(testPageIO database)
target_link_libraries(testPageIO gtest gtest_main)

add_executable(testPage testPage.cpp)
target_link_libraries(testPage database)
target_link_libraries(testPage gtest gtest_main)

add_executable(testManager testManager.cpp)
target_link_libraries(testManager database)
target_link_libraries(testManager gtest gtest_main)
