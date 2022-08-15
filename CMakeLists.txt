cmake_minimum_required(VERSION 3.22)
project(yayo VERSION 0.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -fsanitize=address")
set(CMAKE_CXX_FLAGS "-mbmi2")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#add_compile_options(-fsanitize=address -fno-omit-frame-pointer -mbmi2)
#add_link_options(-fsanitize=address -fno-omit-frame-pointer)

add_executable(yayo main.cpp bitboard.cpp)
