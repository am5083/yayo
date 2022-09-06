yayo: bitboard.cpp board.cpp main.cpp
	g++-12 -o yayo -std=c++20 -mbmi2 -Wall -O3 bitboard.cpp board.cpp main.cpp -I .
