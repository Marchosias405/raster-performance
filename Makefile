CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic

# Default to an unoptimized build.
# Later experiments can override this, for example:
# make OPT=-O3
OPT ?= -O0

BUILD_DIR := build
TARGET := $(BUILD_DIR)/raster-performance
SOURCES := src/main.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OPT) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
