CXX=g++-12
CXXFLAGS=-I. -funroll-loops -O3 -std=c++20 -fopenmp -mbmi2 -Wall -Wextra -pedantic-errors
LDFLAGS=-L /usr/lib/llvm-14/lib/
LDLIBS=-lomp
EXE=yayo

SRC := src
TARGET := $(EXE)
BUILD := build

SEARCHHPP = $(addsuffix /*.hpp ,$(SRC))
SEARCHCPP = $(addsuffix /*.cpp ,$(SRC))
SRCS := $(wildcard $(SEARCHCPP))

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Darwin)
    LDFLAGS = -L /usr/local/opt/libomp/lib/
endif

OBJS := $(subst $(SRC)/,$(BUILD)/,$(addsuffix .o,$(basename $(SRCS))))

$(TARGET): $(OBJS)
	@echo $(SRCS)
	@echo $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD)/%.o: $(SRC)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(BUILD)/*.o
	rm -f $(TARGET)
	rm -rf $(BUILD)/*.o
