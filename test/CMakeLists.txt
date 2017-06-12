project(test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLOGGING")
#set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(libs/googletest)
add_subdirectory(example)
add_subdirectory(buffer_manager)
