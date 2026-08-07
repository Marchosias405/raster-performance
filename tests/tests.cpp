#include "raster.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

// Compare floats with a small tolerance.
// This is safer than relying on exact equality for calculated results.
void assert_close(
    float actual,
    float expected,
    float tolerance = 0.000001f
) {
    assert(std::fabs(actual - expected) <= tolerance);
}

// Flow:
// 1. Generate a small raster with known dimensions.
// 2. Check the dimensions and number of stored cells.
// 3. Check several values that we can calculate by hand.
// 4. Generate the same raster again and confirm it is identical.
// 5. Check that the documented 1000-cell pattern repeats.
void test_synthetic_raster_generation() {
    Raster raster = generate_synthetic_raster(3, 2);

    // 3 columns * 2 rows = 6 cells.
    assert(raster.width == 3);
    assert(raster.height == 2);
    assert(raster.values.size() == 6);

    // Formula:
    // value = (index % 1000) / 1000.0
    assert_close(raster.values[0], 0.000f);
    assert_close(raster.values[1], 0.001f);
    assert_close(raster.values[2], 0.002f);
    assert_close(raster.values[5], 0.005f);

    // The generator must be deterministic.
    Raster repeated = generate_synthetic_raster(3, 2);
    assert(raster.values == repeated.values);

    // The pattern repeats after 1000 cells.
    Raster large_enough = generate_synthetic_raster(1001, 1);

    assert_close(large_enough.values[999], 0.999f);
    assert_close(large_enough.values[1000], 0.000f);
}

// Hand-created test data.
//
// Input:
//  1   2   3
//  4  14   6
//  7   8   9
//
// Center average:
// (1 + 2 + 3 + 4 + 14 + 6 + 7 + 8 + 9) / 9
// = 54 / 9
// = 6
//
// Border cells should stay unchanged.
void test_smooth_row_major_3x3() {
    Raster input{
        3,
        3,
        {
            1.0f,  2.0f, 3.0f,
            4.0f, 14.0f, 6.0f,
            7.0f,  8.0f, 9.0f
        }
    };

    Raster output = smooth_row_major(input);

    assert(output.width == 3);
    assert(output.height == 3);
    assert(output.values.size() == 9);

    const std::vector<float> expected{
        1.0f, 2.0f, 3.0f,
        4.0f, 6.0f, 6.0f,
        7.0f, 8.0f, 9.0f
    };

    for (std::size_t index = 0; index < expected.size(); ++index) {
        assert_close(output.values[index], expected[index]);
    }
}

// A raster smaller than 3x3 has no interior cell.
//
// The smoothing function should therefore return the
// values unchanged.
void test_smooth_row_major_small_raster() {
    Raster input{
        2,
        2,
        {
            1.0f, 2.0f,
            3.0f, 4.0f
        }
    };

    Raster output = smooth_row_major(input);

    assert(output.width == input.width);
    assert(output.height == input.height);
    assert(output.values == input.values);
}

// Hand-created 4x4 test with four interior cells.
//
// Input:
// 0  0  0  0
// 0  9  0  0
// 0  0  0  0
// 0  0  0  0
//
// The value 9 is inside the 3x3 neighborhood of all
// four interior cells.
//
// Therefore each interior average is:
// 9 / 9 = 1
//
// Border cells remain unchanged.
void test_smooth_row_major_multiple_interior_cells() {
    Raster input{
        4,
        4,
        {
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 9.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f
        }
    };

    Raster output = smooth_row_major(input);

    const std::vector<float> expected{
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    };

    for (std::size_t index = 0; index < expected.size(); ++index) {
        assert_close(output.values[index], expected[index]);
    }

    // Smoothing writes to a separate output raster,
    // so the original input must stay unchanged.
    assert_close(input.values[5], 9.0f);
}

int main() {
    test_synthetic_raster_generation();
    test_smooth_row_major_3x3();
    test_smooth_row_major_small_raster();
    test_smooth_row_major_multiple_interior_cells();

    std::cout << "All raster tests passed\n";
    return 0;
}