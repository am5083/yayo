set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -Og")
set(CMAKE_CXX_FLAGS "-std=c++20 -mbmi2 -mbmi -Wall -Wextra -pedantic-errors")

add_executable(yayo main.cpp move.cpp bitboard.cpp board.cpp movegen.cpp eval.cpp tt.cpp thread.cpp uci.cpp)
target_include_directories(yayo PRIVATE ./)
