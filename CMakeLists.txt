cmake_minimum_required(VERSION 3.22)
project(yayo VERSION 0.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3")
set(CMAKE_CXX_FLAGS "-std=c++20 -mbmi2 -mbmi")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(yayo main.cpp bitboard.cpp board.cpp)
