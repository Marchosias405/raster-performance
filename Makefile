CXX := g++

CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic
CPPFLAGS := -Isrc

# Optimization level can be changed from the command line.
#
# Example:
# make OPT=-O3
OPT ?= -O0

# Manual SIMD code uses verified 256-bit AVX instructions.
SIMD_FLAGS := -mavx

BUILD_DIR := build

TARGET := $(BUILD_DIR)/raster-performance
TEST_TARGET := $(BUILD_DIR)/tests

SOURCES := \
	src/main.cpp \
	src/raster.cpp \
	src/raster_simd.cpp

TEST_SOURCES := \
	tests/tests.cpp \
	src/raster.cpp \
	src/raster_simd.cpp

# Default benchmark arguments for "make run".
IMPL ?= row
WIDTH ?= 1024
HEIGHT ?= 1024
REPS ?= 7

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(SOURCES) src/raster.h
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OPT) $(SIMD_FLAGS) $(SOURCES) -o $(TARGET)

$(TEST_TARGET): $(TEST_SOURCES) src/raster.h
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OPT) $(SIMD_FLAGS) $(TEST_SOURCES) -o $(TEST_TARGET)

run: $(TARGET)
	./$(TARGET) $(IMPL) $(WIDTH) $(HEIGHT) $(REPS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR)