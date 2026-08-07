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
