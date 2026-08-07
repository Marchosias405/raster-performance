CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic
CPPFLAGS := -Isrc

# Default to an unoptimized build.
# Later experiments can override this, for example:
# make OPT=-O3
OPT ?= -O0

BUILD_DIR := build

TARGET := $(BUILD_DIR)/raster-performance
TEST_TARGET := $(BUILD_DIR)/tests

SOURCES := src/main.cpp
TEST_SOURCES := tests/tests.cpp src/raster.cpp src/raster_simd.cpp

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OPT) $(SOURCES) -o $(TARGET)

$(TEST_TARGET): $(TEST_SOURCES) src/raster.h
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OPT) -mavx $(TEST_SOURCES) -o $(TEST_TARGET)

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR)
