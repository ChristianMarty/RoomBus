cmake_minimum_required(VERSION 3.24)
project(TestuCodebase)

set(CMAKE_CXX_STANDARD 23)

include_directories(.)

add_executable(TestuCodebase main.cpp  ../utility/string.c )

target_include_directories(TestuCodebase PRIVATE ..)