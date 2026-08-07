#include "raster.h"

// Flow:
// 1. Create a raster with the requested width and height.
// 2. Allocate one float for every raster cell.
// 3. Generate each value from its 1D array index.
// 4. Return the completed synthetic raster.
//
// Dataset provenance:
// These values are generated entirely by this project's code.
// No external GIS dataset is used.
Raster generate_synthetic_raster(
    std::size_t width,
    std::size_t height
) {
    Raster raster;

    raster.width = width;
    raster.height = height;

    const std::size_t total_cells = width * height;
    raster.values.resize(total_cells);

    for (std::size_t index = 0; index < total_cells; ++index) {
        // Produce deterministic values from 0.000 to 0.999.
        // The pattern repeats every 1000 cells.
        raster.values[index] =
            static_cast<float>(index % 1000) / 1000.0f;
    }

    return raster;
}

// Flow:
// 1. Copy the input so all border cells stay unchanged.
// 2. Move through interior cells row by row.
// 3. Add the 9 values in each 3x3 neighborhood.
// 4. Divide the sum by 9.
// 5. Store the average in the output raster.
//
// Memory layout:
// row * width + column converts a 2D coordinate
// into its position in the contiguous 1D array.
Raster smooth_row_major(const Raster& input) {
    Raster output = input;

    // A raster smaller than 3x3 has no interior cells.
    // Returning the copy keeps every value unchanged.
    if (input.width < 3 || input.height < 3) {
        return output;
    }

    for (std::size_t row = 1; row < input.height - 1; ++row) {
        for (std::size_t column = 1; column < input.width - 1; ++column) {
            const std::size_t top = (row - 1) * input.width + column;
            const std::size_t middle = row * input.width + column;
            const std::size_t bottom = (row + 1) * input.width + column;

            const float sum =
                input.values[top - 1] +
                input.values[top] +
                input.values[top + 1] +
                input.values[middle - 1] +
                input.values[middle] +
                input.values[middle + 1] +
                input.values[bottom - 1] +
                input.values[bottom] +
                input.values[bottom + 1];

            output.values[middle] = sum / 9.0f;
        }
    }

    return output;
}

// Flow:
// 1. Copy the input so all border cells stay unchanged.
// 2. Move through interior cells column by column.
// 3. Add the 9 values in each 3x3 neighborhood.
// 4. Divide the sum by 9.
// 5. Store the average in the output raster.
//
// Memory experiment:
// The raster is still stored in row-major contiguous memory.
// Only the traversal order changes compared with smooth_row_major().
Raster smooth_column_major(const Raster& input) {
    Raster output = input;

    // A raster smaller than 3x3 has no interior cells.
    if (input.width < 3 || input.height < 3) {
        return output;
    }

    for (std::size_t column = 1; column < input.width - 1; ++column) {
        for (std::size_t row = 1; row < input.height - 1; ++row) {
            const std::size_t top = (row - 1) * input.width + column;
            const std::size_t middle = row * input.width + column;
            const std::size_t bottom = (row + 1) * input.width + column;

            const float sum =
                input.values[top - 1] +
                input.values[top] +
                input.values[top + 1] +
                input.values[middle - 1] +
                input.values[middle] +
                input.values[middle + 1] +
                input.values[bottom - 1] +
                input.values[bottom] +
                input.values[bottom + 1];

            output.values[middle] = sum / 9.0f;
        }
    }

    return output;
}
