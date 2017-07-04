project(database)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLOGGING")

set(buffer_SOURCE_FILES
        buffer/Page.h buffer/Page.cpp
        buffer/PageIO.h buffer/PageIO.cpp
        buffer/BufferManager.h buffer/BufferManager.cpp
        buffer/BufferFrame.h buffer/BufferFrame.cpp)

set(slotted_SOURCE_FILES
        slotted/Record.h
        slotted/Segment.h
        slotted/SPSegment.cpp slotted/SPSegment.h
        slotted/TID.h)

set(bplus_SOURCE_FILES
        index/BPlusTree.h)

set(operators_SOURCE_FILES
        operators/Register.h
        operators/Operator.h operators/TableScan.cpp operators/TableScan.h operators/Print.cpp operators/Print.h operators/Projection.cpp operators/Projection.h)

set(relations_SOURCE_FILES
        relations/Type.h
        relations/Schema.h)

set(utils_SOURCE_FILES
        utils/LockGuards.h)

add_library(database
        ${buffer_SOURCE_FILES}
        ${slotted_SOURCE_FILES}
        ${bplus_SOURCE_FILES}
        ${utils_SOURCE_FILES}
        ${operators_SOURCE_FILES}
        ${relations_SOURCE_FILES}
        dbms.h)
