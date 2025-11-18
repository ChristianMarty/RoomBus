cmake_minimum_required(VERSION 3.24)
project(kernel)

set(CMAKE_CXX_STANDARD 23)

include_directories(kernel/)
include_directories(../uCodebase)

add_executable(kernel kernel/main.cpp)
