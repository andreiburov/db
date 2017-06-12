project(database)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLOGGING")
#set(CMAKE_BUILD_TYPE Debug)

set(buffer_manager_SOURCE_FILES
        buffer_manager/buffer_manager.h buffer_manager/buffer_manager.cpp
        buffer_manager/buffer_frame.h buffer_manager/buffer_frame.cpp
        buffer_manager/segment.h buffer_manager/segment.cpp
        buffer_manager/page_size.h buffer_manager/page_size.cpp
        buffer_manager/list.h buffer_manager/list.cpp
        buffer_manager/no_slot_exception.h buffer/PageIO.cpp buffer/PageIO.h)

set(buffer_SOURCE_FILES
        buffer/Page.h buffer/Page.cpp
        buffer/PageIO.h buffer/PageIO.cpp
        buffer/BufferManager.h buffer/BufferManager.cpp
        buffer/BufferFrame.h buffer/BufferFrame.cpp)

set(utils_SOURCE_FILES
        utils/debug.h
        utils/lockguards.h buffer/BufferFrame.cpp buffer/BufferFrame.h)

add_library(database
        ${buffer_SOURCE_FILES}
        ${utils_SOURCE_FILES})
