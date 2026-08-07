#ifndef RASTER_H
#define RASTER_H

#include <cstddef>
#include <vector>

// A raster is stored as one contiguous 1D array.
//
// Memory layout:
// - width  = number of columns
// - height = number of rows
// - values stores the cells in row-major order
//
// Later, a cell at (row, column) can be accessed with:
// row * width + column
struct Raster {
    std::size_t width;
    std::size_t height;
    std::vector<float> values;
};

// Generate deterministic synthetic GIS-style raster data.
//
// The exact generation formula will be implemented and documented
// in raster.cpp. The same dimensions must produce the same values.
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

#endif
