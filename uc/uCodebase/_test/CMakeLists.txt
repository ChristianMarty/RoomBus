cmake_minimum_required(VERSION 3.24)
project(TestuCodebase)

set(CMAKE_CXX_STANDARD 23)

include_directories(.)
include_directories(../)

add_executable(TestuCodebase
        main.cpp
        bus.c
        ../utility/string.c
        ../utility/perlin.c
        ../utility/iniParser/iniParser.c
        ../driver/SAMx5x/tickTimer.c
        ../protocol/valueSystemProtocol.cpp
)
        test_ringbufferU8.hpp test_iniParser.h)

target_include_directories(TestuCodebase PRIVATE ..)