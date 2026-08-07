#include "raster.h"

#include <algorithm>
#include <random>
#include <stdexcept>

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
// 1. Assume output storage is already allocated and initialized.
// 2. Move through interior cells row by row.
// 3. Add the 9 values in each 3x3 neighborhood.
// 4. Divide the sum by 9.
// 5. Store the average in the existing output raster.
//
// Benchmark purpose:
// Allocation and border initialization can happen before timing.
//
// Precondition:
// output has the same dimensions and number of cells as input.
void smooth_row_major_into(
    const Raster& input,
    Raster& output
) {
    // A raster smaller than 3x3 has no interior cells.
    if (input.width < 3 || input.height < 3) {
        return;
    }

    // Row is the outer loop.
    for (std::size_t row = 1;
         row < input.height - 1;
         ++row) {

        // Column is the inner loop.
        for (std::size_t column = 1;
             column < input.width - 1;
             ++column) {

            const std::size_t top =
                (row - 1) * input.width + column;

            const std::size_t middle =
                row * input.width + column;

            const std::size_t bottom =
                (row + 1) * input.width + column;

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
}

// Correctness-friendly wrapper.
//
// Copying the input preserves the established border policy.
// Benchmarks will call smooth_row_major_into() directly so
// this copy does not occur inside the timed kernel.
Raster smooth_row_major(const Raster& input) {
    Raster output = input;

    smooth_row_major_into(input, output);

    return output;
}

// Flow:
// 1. Assume output storage is already allocated and initialized.
// 2. Move through interior cells column by column.
// 3. Add the 9 values in each 3x3 neighborhood.
// 4. Divide the sum by 9.
// 5. Store the average in the existing output raster.
//
// Benchmark purpose:
// Allocation and border initialization can happen before timing.
//
// The arithmetic is the same as row-major smoothing.
// Only the traversal order changes.
//
// Precondition:
// output has the same dimensions and number of cells as input.
void smooth_column_major_into(
    const Raster& input,
    Raster& output
) {
    // A raster smaller than 3x3 has no interior cells.
    if (input.width < 3 || input.height < 3) {
        return;
    }

    // Column is the outer loop.
    for (std::size_t column = 1;
         column < input.width - 1;
         ++column) {

        // Row is the inner loop.
        for (std::size_t row = 1;
             row < input.height - 1;
             ++row) {

            const std::size_t top =
                (row - 1) * input.width + column;

            const std::size_t middle =
                row * input.width + column;

            const std::size_t bottom =
                (row + 1) * input.width + column;

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
}

// Correctness-friendly wrapper.
//
// Copying the input preserves the border cells.
// Benchmarks will call smooth_column_major_into()
// directly so this copy is outside the timed region.
Raster smooth_column_major(const Raster& input) {
    Raster output = input;

    smooth_column_major_into(input, output);

    return output;
}

// Flow:
// 1. Check that the requested passing percentage is valid.
// 2. Create enough cells for width * height.
// 3. Fill the raster with failing values (0.25f).
// 4. Replace the required number of cells with passing values (0.75f).
// 5. Optionally shuffle the same values using a fixed seed.
// 6. Return the controlled synthetic raster.
//
// Dataset provenance:
// This data is generated entirely by this project's code.
// No external dataset is used.
//
// For totals divisible by 100, pass_percent gives an exact
// percentage. Otherwise integer division rounds the passing
// cell count down.
Raster generate_threshold_raster(
    std::size_t width,
    std::size_t height,
    std::size_t pass_percent,
    bool shuffled,
    std::uint32_t seed
) {
    if (pass_percent > 100) {
        throw std::invalid_argument(
            "pass_percent must be between 0 and 100"
        );
    }

    Raster raster;
    raster.width = width;
    raster.height = height;

    const std::size_t total_cells = width * height;

    const std::size_t pass_count =
        (total_cells * pass_percent) / 100;

    // Start with every cell below the later 0.5f threshold.
    raster.values.assign(total_cells, 0.25f);

    // Keep failing values first and passing values last.
    // This gives us the grouped input when shuffled == false.
    const std::size_t first_passing =
        total_cells - pass_count;

    for (std::size_t index = first_passing;
         index < total_cells;
         ++index) {

        raster.values[index] = 0.75f;
    }

    if (shuffled) {
        // Fixed-seed mt19937 makes the shuffled order reproducible.
        std::mt19937 generator(seed);

        std::shuffle(
            raster.values.begin(),
            raster.values.end(),
            generator
        );
    }

    return raster;
}

// Flow:
// 1. Create one output byte for every input cell.
// 2. Check each value against the threshold.
// 3. Explicitly choose 1 or 0 with if/else.
// 4. Return the classification results.
//
// Comparison rule:
// A value passes only when value > threshold.
std::vector<std::uint8_t> classify_branch(
    const Raster& input,
    float threshold
) {
    std::vector<std::uint8_t> output(
        input.values.size()
    );

    for (std::size_t index = 0;
         index < input.values.size();
         ++index) {

        if (input.values[index] > threshold) {
            output[index] = 1;
        } else {
            output[index] = 0;
        }
    }

    return output;
}

// Flow:
// 1. Create one output byte for every input cell.
// 2. Evaluate value > threshold.
// 3. Convert the boolean result to 0 or 1.
// 4. Return the classification results.
//
// This source code contains no explicit if/else.
//
// We will inspect generated assembly later before deciding
// whether the resulting machine code is actually branchless.
std::vector<std::uint8_t> classify_branchless(
    const Raster& input,
    float threshold
) {
    std::vector<std::uint8_t> output(
        input.values.size()
    );

    for (std::size_t index = 0;
         index < input.values.size();
         ++index) {

        output[index] =
            static_cast<std::uint8_t>(
                input.values[index] > threshold
            );
    }

    return output;
}