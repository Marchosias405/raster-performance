#ifndef RASTER_H
#define RASTER_H

#include <cstddef>
#include <cstdint>
#include <vector>

// A raster is stored as one contiguous 1D array.
//
// Memory layout:
// - width  = number of columns
// - height = number of rows
// - values stores the cells in row-major order
//
// A cell at (row, column) is located with:
// row * width + column
struct Raster {
    std::size_t width;
    std::size_t height;
    std::vector<float> values;
};

// Generate deterministic synthetic GIS-style raster data.
//
// The same dimensions always produce the same values.
Raster generate_synthetic_raster(
    std::size_t width,
    std::size_t height
);

// Apply 3x3 average smoothing in row-major traversal.
//
// Border policy:
// Border cells are copied unchanged.
//
// Each interior output cell is the average of its
// 3x3 neighborhood from the input raster.
Raster smooth_row_major(const Raster& input);

// Apply the same 3x3 average smoothing using
// column-major traversal.
//
// The raster is still stored in row-major contiguous memory.
// Only the loop traversal order changes.
//
// Border cells are copied unchanged.
Raster smooth_column_major(const Raster& input);

// Apply the same 3x3 average smoothing using
// 256-bit AVX SIMD operations.
//
// Eight adjacent float output cells can be processed
// together in one vector iteration.
//
// Border cells are copied unchanged.
// Remaining interior cells are handled by a scalar tail loop.
Raster smooth_simd_avx(const Raster& input);

// Benchmark-oriented smoothing versions.
//
// The caller provides an already allocated and initialized
// output raster.
//
// These functions update only the interior cells so that
// allocation and border initialization can happen outside
// the timed benchmark region.
void smooth_row_major_into(
    const Raster& input,
    Raster& output
);

void smooth_column_major_into(
    const Raster& input,
    Raster& output
);

void smooth_simd_avx_into(
    const Raster& input,
    Raster& output
);

// Generate controlled synthetic data for branch experiments.
//
// Values that do not pass are 0.25f.
// Values that pass are 0.75f.
// The later classification threshold will be 0.5f.
//
// pass_percent must be from 0 through 100.
//
// When shuffled is false:
// - failing values come first
// - passing values are grouped at the end
//
// When shuffled is true:
// - the same values are shuffled
// - std::mt19937 uses the supplied fixed seed
Raster generate_threshold_raster(
    std::size_t width,
    std::size_t height,
    std::size_t pass_percent,
    bool shuffled,
    std::uint32_t seed
);

// Explicit if/else classification.
//
// A cell passes only when:
// value > threshold
//
// Output:
// 0 = does not pass
// 1 = passes
std::vector<std::uint8_t> classify_branch(
    const Raster& input,
    float threshold
);

// Source-level branchless classification.
//
// This source version has no explicit if/else.
//
// We do not assume this becomes branchless machine code.
// Generated assembly will be inspected later.
std::vector<std::uint8_t> classify_branchless(
    const Raster& input,
    float threshold
);

// Benchmark-oriented classification versions.
//
// The caller provides already allocated output storage,
// so allocation can happen outside the timed region.
//
// Each output byte is:
// 0 = value does not pass
// 1 = value passes
void classify_branch_into(
    const Raster& input,
    float threshold,
    std::vector<std::uint8_t>& output
);

void classify_branchless_into(
    const Raster& input,
    float threshold,
    std::vector<std::uint8_t>& output
);

#endif