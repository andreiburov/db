cmake_minimum_required(VERSION 2.8)

project(db)
add_executable(${PROJECT_NAME} "main.cpp")

add_subdirectory(database)
add_subdirectory(test)

