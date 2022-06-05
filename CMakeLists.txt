cmake_minimum_required(VERSION 3.6)

project(threadMessage)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -O0 -g)

include_directories(include gtest)

file(GLOB_RECURSE src_files "source/*.cpp")
file(GLOB_RECURSE src_files_unit_test "unitTest/*.cpp")

link_directories(gtest/lib)

add_executable(${PROJECT_NAME} main/main.cpp ${src_files} ${src_files_unit_test})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "threadMessage")

target_link_libraries(${PROJECT_NAME} stdc++ pthread gmock gtest)