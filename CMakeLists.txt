cmake_minimum_required(VERSION 3.19)
project(PageMin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "-O2")

add_executable(PageMin
        src/main.cpp
        src/PageMin.cpp
        src/Node.cpp)

    target_include_directories(PageMin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
