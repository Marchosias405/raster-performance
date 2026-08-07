#include "raster.h"

#include <immintrin.h>

// Flow:
// 1. Copy the input so border cells stay unchanged.
// 2. Process each interior row.
// 3. Calculate 8 adjacent output cells with one 256-bit AVX vector.
// 4. Each vector lane adds one complete 3x3 neighborhood.
// 5. Divide all 8 sums by 9.
// 6. Store the 8 results.
// 7. Finish any remaining interior cells with a scalar tail loop.
//
// SIMD layout:
// 256 bits / 32 bits per float = 8 float lanes.
//
// _mm256_loadu_ps is used because the starting addresses are
// not guaranteed to be aligned to a 32-byte boundary.
Raster smooth_simd_avx(const Raster& input) {
    Raster output = input;

    // A raster smaller than 3x3 has no interior cells.
    if (input.width < 3 || input.height < 3) {
        return output;
    }

    const __m256 nine = _mm256_set1_ps(9.0f);

    for (std::size_t row = 1; row < input.height - 1; ++row) {
        std::size_t column = 1;

        // Interior columns are:
        // 1 through width - 2.
        //
        // A vector iteration needs 8 consecutive output cells.
        // column + 8 <= width - 1 guarantees that:
        // - all 8 outputs are interior cells
        // - the rightmost 3x3 read reaches at most the border
        for (;
             column + 8 <= input.width - 1;
             column += 8) {

            const std::size_t top =
                (row - 1) * input.width + column;

            const std::size_t middle =
                row * input.width + column;

            const std::size_t bottom =
                (row + 1) * input.width + column;

            // Load 8 adjacent floats from each horizontal
            // position of the three raster rows.
            //
            // Together these nine vectors represent the
            // 3x3 neighborhoods for 8 output cells.
            const __m256 top_left =
                _mm256_loadu_ps(&input.values[top - 1]);

            const __m256 top_center =
                _mm256_loadu_ps(&input.values[top]);

            const __m256 top_right =
                _mm256_loadu_ps(&input.values[top + 1]);

            const __m256 middle_left =
                _mm256_loadu_ps(&input.values[middle - 1]);

            const __m256 middle_center =
                _mm256_loadu_ps(&input.values[middle]);

            const __m256 middle_right =
                _mm256_loadu_ps(&input.values[middle + 1]);

            const __m256 bottom_left =
                _mm256_loadu_ps(&input.values[bottom - 1]);

            const __m256 bottom_center =
                _mm256_loadu_ps(&input.values[bottom]);

            const __m256 bottom_right =
                _mm256_loadu_ps(&input.values[bottom + 1]);

            // Add the nine neighborhoods lane by lane.
            __m256 sum = _mm256_add_ps(top_left, top_center);
            sum = _mm256_add_ps(sum, top_right);
            sum = _mm256_add_ps(sum, middle_left);
            sum = _mm256_add_ps(sum, middle_center);
            sum = _mm256_add_ps(sum, middle_right);
            sum = _mm256_add_ps(sum, bottom_left);
            sum = _mm256_add_ps(sum, bottom_center);
            sum = _mm256_add_ps(sum, bottom_right);

            // Divide all 8 neighborhood sums by 9.
            const __m256 average = _mm256_div_ps(sum, nine);

            // Store 8 adjacent smoothed output cells.
            _mm256_storeu_ps(
                &output.values[middle],
                average
            );
        }

        // Scalar tail:
        // Handle interior cells left over after groups of 8.
        for (; column < input.width - 1; ++column) {
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

    return output;
}