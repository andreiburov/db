project(database)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(DATABASE_SOURCE_FILES
  database.h database.cpp)

add_library(database
  ${DATABASE_SOURCE_FILES})
