set (CMAKE_CXX_COMPILER "/usr/local/bin/g++-12")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.22)
project(yayo VERSION 0.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -Og")
set(CMAKE_CXX_FLAGS "-std=c++20 -mbmi2 -mbmi")

add_executable(yayo main.cpp move.cpp bitboard.cpp board.cpp movegen.cpp eval.cpp tt.cpp thread.cpp uci.cpp)
target_include_directories(yayo PRIVATE ./)
