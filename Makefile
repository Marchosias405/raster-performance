CXX := g++

CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic
CPPFLAGS := -Isrc

# Optimization level can be changed from the command line.
#
# Example:
# make OPT=-O3
#
# Branch-prediction experiment:
# make branch OPT="-O3 -fno-tree-vectorize"
OPT ?= -O0

# Manual SIMD code uses verified 256-bit AVX instructions.
SIMD_FLAGS := -mavx

BUILD_DIR := build

TARGET := $(BUILD_DIR)/raster-performance
TEST_TARGET := $(BUILD_DIR)/tests
BRANCH_TARGET := $(BUILD_DIR)/branch-benchmark

SOURCES := \
	src/main.cpp \
	src/raster.cpp \
	src/raster_simd.cpp

TEST_SOURCES := \
	tests/tests.cpp \
	src/raster.cpp \
	src/raster_simd.cpp

BRANCH_SOURCES := \
	src/branch_benchmark.cpp \
	src/raster.cpp

# Default smoothing benchmark arguments for "make run".
IMPL ?= row
WIDTH ?= 1024
HEIGHT ?= 1024
REPS ?= 7

# Default branch benchmark arguments for "make branch-run".
BRANCH_IMPL ?= branch
PASS_PERCENT ?= 50
ORDER ?= shuffled
BRANCH_REPS ?= 7

.PHONY: all run test branch branch-run clean

all: $(TARGET)

$(TARGET): $(SOURCES) src/raster.h
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OPT) $(SIMD_FLAGS) $(SOURCES) -o $(TARGET)

$(TEST_TARGET): $(TEST_SOURCES) src/raster.h
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OPT) $(SIMD_FLAGS) $(TEST_SOURCES) -o $(TEST_TARGET)

$(BRANCH_TARGET): $(BRANCH_SOURCES) src/raster.h
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OPT) $(SIMD_FLAGS) $(BRANCH_SOURCES) -o $(BRANCH_TARGET)

run: $(TARGET)
	./$(TARGET) $(IMPL) $(WIDTH) $(HEIGHT) $(REPS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

branch: $(BRANCH_TARGET)

branch-run: $(BRANCH_TARGET)
	./$(BRANCH_TARGET) $(BRANCH_IMPL) $(WIDTH) $(HEIGHT) $(PASS_PERCENT) $(ORDER) $(BRANCH_REPS)

clean:
	rm -rf $(BUILD_DIR)