#include "raster.h"

#include <cassert>
#include <iostream>

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
    assert(raster.values[0] == 0.000f);
    assert(raster.values[1] == 0.001f);
    assert(raster.values[2] == 0.002f);
    assert(raster.values[5] == 0.005f);

    // The generator must be deterministic.
    Raster repeated = generate_synthetic_raster(3, 2);
    assert(raster.values == repeated.values);

    // The pattern repeats after 1000 cells.
    Raster large_enough = generate_synthetic_raster(1001, 1);

    assert(large_enough.values[999] == 0.999f);
    assert(large_enough.values[1000] == 0.000f);
}

int main() {
    test_synthetic_raster_generation();

    std::cout << "All raster generation tests passed\n";
    return 0;
}
