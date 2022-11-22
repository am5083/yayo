set (CMAKE_CXX_COMPILER "/usr/local/bin/g++-12")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.22)
project(yayo VERSION 0.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -fsanitize=address,undefined -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -funroll-loops")
set(CMAKE_CXX_FLAGS "-std=c++20 -mbmi2 -mbmi")

add_executable(
  yayo
  ${CMAKE_SOURCE_DIR}/src/main.cpp
  ${CMAKE_SOURCE_DIR}/src/move.cpp
  ${CMAKE_SOURCE_DIR}/src/bitboard.cpp
  ${CMAKE_SOURCE_DIR}/src/board.cpp
  ${CMAKE_SOURCE_DIR}/src/movegen.cpp
  ${CMAKE_SOURCE_DIR}/src/eval.cpp
  ${CMAKE_SOURCE_DIR}/src/tt.cpp
  ${CMAKE_SOURCE_DIR}/src/thread.cpp
  ${CMAKE_SOURCE_DIR}/src/uci.cpp
  ${CMAKE_SOURCE_DIR}/src/tuner.cpp
  )

target_include_directories(yayo PRIVATE ./)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(yayo PUBLIC OpenMP::OpenMP_CXX)
endif()
