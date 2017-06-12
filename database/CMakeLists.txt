project(database)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLOGGING")

set(buffer_SOURCE_FILES
        buffer/Page.h buffer/Page.cpp
        buffer/PageIO.h buffer/PageIO.cpp
        buffer/BufferManager.h buffer/BufferManager.cpp
        buffer/BufferFrame.h buffer/BufferFrame.cpp)

set(slotted_pages_SOURCE_FILES
        slotted_pages/Record.h)

set(utils_SOURCE_FILES
        utils/lockguards.h)

add_library(database
        ${buffer_SOURCE_FILES}
        ${slotted_pages_SOURCE_FILES}
        ${utils_SOURCE_FILES} dbms.h)
