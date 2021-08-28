cmake_minimum_required(VERSION 3.19)
project(MinPage-Tree)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "-O2")

add_executable(MPT
        src/main.cpp
        src/MPT.cpp
        src/Node.cpp)

target_include_directories(MPT PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
