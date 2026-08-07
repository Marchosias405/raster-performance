#include "raster.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include <stdexcept>

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

// Verify that changing traversal order does not change
// the mathematical smoothing result.
//
// The row-major implementation is our verified baseline.
// Column-major should produce the same output values.
void test_smooth_column_major_matches_row_major() {
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

    Raster row_output = smooth_row_major(input);
    Raster column_output = smooth_column_major(input);

    assert(row_output.width == column_output.width);
    assert(row_output.height == column_output.height);
    assert(row_output.values.size() == column_output.values.size());

    for (std::size_t index = 0;
         index < row_output.values.size();
         ++index) {
        assert_close(
            column_output.values[index],
            row_output.values[index]
        );
    }
}

// Check equivalence on a larger generated raster with
// multiple rows and columns of interior cells.
void test_smooth_column_major_generated_raster() {
    Raster input = generate_synthetic_raster(7, 6);

    Raster row_output = smooth_row_major(input);
    Raster column_output = smooth_column_major(input);

    assert(row_output.values.size() == column_output.values.size());

    for (std::size_t index = 0;
         index < row_output.values.size();
         ++index) {
        assert_close(
            column_output.values[index],
            row_output.values[index]
        );
    }
}

// Count how many raster values equal a chosen value.
// Threshold datasets contain exact 0.25f and 0.75f values.
std::size_t count_value(
    const Raster& raster,
    float value
) {
    std::size_t count = 0;

    for (float cell : raster.values) {
        if (cell == value) {
            ++count;
        }
    }

    return count;
}

// Verify exact 5%, 50%, and 95% passing counts.
//
// We use exactly 100 cells so the requested percentages
// correspond directly to exact cell counts.
void test_threshold_raster_percentages() {
    Raster five_percent =
        generate_threshold_raster(10, 10, 5, false, 12345);

    Raster fifty_percent =
        generate_threshold_raster(10, 10, 50, false, 12345);

    Raster ninety_five_percent =
        generate_threshold_raster(10, 10, 95, false, 12345);

    assert(five_percent.values.size() == 100);
    assert(count_value(five_percent, 0.25f) == 95);
    assert(count_value(five_percent, 0.75f) == 5);

    assert(count_value(fifty_percent, 0.25f) == 50);
    assert(count_value(fifty_percent, 0.75f) == 50);

    assert(count_value(ninety_five_percent, 0.25f) == 5);
    assert(count_value(ninety_five_percent, 0.75f) == 95);
}

// Verify the grouped 50/50 pattern.
//
// Expected:
// first 50 cells  = 0.25f
// final 50 cells  = 0.75f
void test_threshold_raster_grouped_order() {
    Raster grouped =
        generate_threshold_raster(10, 10, 50, false, 12345);

    for (std::size_t index = 0; index < 50; ++index) {
        assert(grouped.values[index] == 0.25f);
    }

    for (std::size_t index = 50; index < 100; ++index) {
        assert(grouped.values[index] == 0.75f);
    }
}

// Verify that shuffling is deterministic.
//
// The same values and same fixed seed must produce
// the same shuffled order each time.
void test_threshold_raster_deterministic_shuffle() {
    constexpr std::uint32_t seed = 12345;

    Raster first =
        generate_threshold_raster(10, 10, 50, true, seed);

    Raster second =
        generate_threshold_raster(10, 10, 50, true, seed);

    assert(first.values == second.values);

    // Shuffling must not change how many values pass.
    assert(count_value(first, 0.25f) == 50);
    assert(count_value(first, 0.75f) == 50);

    Raster grouped =
        generate_threshold_raster(10, 10, 50, false, seed);

    // For this fixed test seed, verify that the shuffled
    // ordering differs from the grouped ordering.
    assert(first.values != grouped.values);
}

// Percentages above 100 are invalid.
void test_threshold_raster_invalid_percentage() {
    bool threw_exception = false;

    try {
        generate_threshold_raster(
            10,
            10,
            101,
            false,
            12345
        );
    } catch (const std::invalid_argument&) {
        threw_exception = true;
    }

    assert(threw_exception);
}

// Verify the exact threshold comparison rule.
//
// threshold = 0.5
//
// 0.25 < 0.5  -> 0
// 0.50 == 0.5 -> 0
// 0.75 > 0.5  -> 1
//
// The value exactly equal to the threshold is important:
// our rule is >, not >=.
void test_classification_threshold_rule() {
    Raster input{
        5,
        1,
        {
            0.25f,
            0.50f,
            0.75f,
            -1.0f,
            1.0f
        }
    };

    const std::vector<std::uint8_t> expected{
        0, 0, 1, 0, 1
    };

    const std::vector<std::uint8_t> branch_output =
        classify_branch(input, 0.5f);

    const std::vector<std::uint8_t> branchless_output =
        classify_branchless(input, 0.5f);

    assert(branch_output == expected);
    assert(branchless_output == expected);
}

// Verify simple edge distributions:
// every value below the threshold and every value above it.
void test_classification_all_below_and_above() {
    Raster all_below{
        4,
        1,
        {
            0.10f,
            0.20f,
            0.30f,
            0.40f
        }
    };

    Raster all_above{
        4,
        1,
        {
            0.60f,
            0.70f,
            0.80f,
            0.90f
        }
    };

    const std::vector<std::uint8_t> expected_below{
        0, 0, 0, 0
    };

    const std::vector<std::uint8_t> expected_above{
        1, 1, 1, 1
    };

    assert(classify_branch(all_below, 0.5f) == expected_below);
    assert(classify_branchless(all_below, 0.5f) == expected_below);

    assert(classify_branch(all_above, 0.5f) == expected_above);
    assert(classify_branchless(all_above, 0.5f) == expected_above);
}

// Count how many classification outputs contain 1.
std::size_t count_classified_true(
    const std::vector<std::uint8_t>& output
) {
    std::size_t count = 0;

    for (std::uint8_t value : output) {
        if (value == 1) {
            ++count;
        }
    }

    return count;
}

// Verify both classification implementations on the
// controlled 5%, 50%, and 95% branch datasets.
void test_classification_controlled_distributions() {
    const std::size_t percentages[]{
        5,
        50,
        95
    };

    for (std::size_t pass_percent : percentages) {
        Raster input = generate_threshold_raster(
            10,
            10,
            pass_percent,
            true,
            12345
        );

        std::vector<std::uint8_t> branch_output =
            classify_branch(input, 0.5f);

        std::vector<std::uint8_t> branchless_output =
            classify_branchless(input, 0.5f);

        // Different source styles must give the same answer.
        assert(branch_output == branchless_output);

        // There are exactly 100 cells, so the expected number
        // of true classifications equals pass_percent.
        assert(
            count_classified_true(branch_output) ==
            pass_percent
        );
    }
}

// Grouped and shuffled 50/50 inputs contain the same
// values in different orders.
//
// Both should therefore still classify exactly 50 cells as true.
void test_classification_grouped_and_shuffled() {
    constexpr std::uint32_t seed = 12345;

    Raster grouped =
        generate_threshold_raster(10, 10, 50, false, seed);

    Raster shuffled =
        generate_threshold_raster(10, 10, 50, true, seed);

    std::vector<std::uint8_t> grouped_output =
        classify_branch(grouped, 0.5f);

    std::vector<std::uint8_t> shuffled_output =
        classify_branch(shuffled, 0.5f);

    assert(count_classified_true(grouped_output) == 50);
    assert(count_classified_true(shuffled_output) == 50);

    assert(
        classify_branch(grouped, 0.5f) ==
        classify_branchless(grouped, 0.5f)
    );

    assert(
        classify_branch(shuffled, 0.5f) ==
        classify_branchless(shuffled, 0.5f)
    );
}

int main() {
    test_synthetic_raster_generation();
    test_smooth_row_major_3x3();
    test_smooth_row_major_small_raster();
    test_smooth_row_major_multiple_interior_cells();

    test_smooth_column_major_matches_row_major();
    test_smooth_column_major_generated_raster();

    test_threshold_raster_percentages();
    test_threshold_raster_grouped_order();
    test_threshold_raster_deterministic_shuffle();
    test_threshold_raster_invalid_percentage();

    test_classification_threshold_rule();
    test_classification_all_below_and_above();
    test_classification_controlled_distributions();
    test_classification_grouped_and_shuffled();

    std::cout << "All raster tests passed\n";
    return 0;
}