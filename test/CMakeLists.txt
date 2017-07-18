project(test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLOGGING")

add_subdirectory(libs/googletest)
add_subdirectory(buffer)
add_subdirectory(slotted)
add_subdirectory(index)
add_subdirectory(operators)
add_subdirectory(util)