cmake_minimum_required(VERSION 3.9.1)
project(CMakeHello)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
add_executable(cmake_hello main.cpp)
