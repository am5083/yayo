# OpenBench-compatible Makefile.
#   make                       -> builds ./yayo
#   make EXE=Yayo-abc CXX=g++  -> builds ./Yayo-abc with the given compiler
# The engine uses the pext instruction at run time, so BMI2 is required.

EXE ?= yayo
CXX ?= g++

SRC  := src
OBJ  := obj
SRCS := $(wildcard $(SRC)/*.cpp)
OBJS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(SRCS))

CXXFLAGS ?= -std=c++20 -O3 -funroll-loops -flto -mbmi -mbmi2 -mpopcnt -march=native -fopenmp -DNDEBUG -w
LDFLAGS  ?= -flto -fopenmp -pthread

.PHONY: all clean
all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cpp $(wildcard $(SRC)/*.hpp) $(wildcard $(SRC)/*.h)
	@mkdir -p $(OBJ)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ) $(EXE)
