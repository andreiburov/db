cmake_minimum_required(VERSION 3.2)
project(subscripts)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

if (NOT WIN32 OR MSYS OR CYGWIN)
    find_program(LLVM_CONFIG_EXECUTABLE "llvm-config")

    if (LLVM_CONFIG_EXECUTABLE STREQUAL "LLVM_CONFIG_EXECUTABLE-NOTFOUND")
        message(FATAL_ERROR "llvm-config could not be found!")
    endif()

    execute_process(
            COMMAND ${LLVM_CONFIG_EXECUTABLE} --cxxflags
            OUTPUT_VARIABLE LLVM_CXXFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
            COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
            OUTPUT_VARIABLE LLVM_LDFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
            COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs mc mcjit core executionengine interpreter support nativecodegen
            OUTPUT_VARIABLE LLVM_LIBS
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(CLANG_LIBS ${LLVM_LIBS} -lffi -ldl)
    endif()

endif()

add_library(subscripts arithmetic/ArithmeticModule.cpp arithmetic/ArithmeticModule.h)
target_link_libraries(subscripts ${LLVM_LDFLAGS})
target_link_libraries(subscripts ${CLANG_LIBS})

set_target_properties(subscripts PROPERTIES COMPILE_FLAGS "${CXXFLAGS} ${LLVM_CXXFLAGS}")