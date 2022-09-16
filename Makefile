CXX=g++-12
CXXFLAGS=-O3 -std=c++20 -mbmi2 -Wall -Wextra -pedantic-errors
EXE=yayo

SRC := src
TARGET := $(EXE)
BUILD := build

SEARCHCPP = $(addsuffix /*.cpp ,$(SRC))
SRCS := $(wildcard $(SEARCHCPP))

OBJS := $(subst $(SRC)/,$(BUILD)/,$(addsuffix .o,$(basename $(SRCS))))

$(TARGET): $(OBJS)
	@echo $(SRCS)
	@echo $(OBJS)
	$(CXX) $(OBJS) -o $@

$(BUILD)/%.o: $(SRC)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(BUILD)/*.o
	rm -f $(TARGET)
