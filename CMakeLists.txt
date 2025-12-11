cmake_minimum_required(VERSION 3.10)
project(SwiftNode)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(SwiftNode src/main.cpp)
