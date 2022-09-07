CXX=g++-12
CXXFLAGS=-I.
EXE=yayo

default: bitboard.cpp board.cpp main.cpp
	g++-12 -o $(EXE) -std=c++20 -mbmi2 -Wall -O3 bitboard.cpp board.cpp main.cpp -I .
