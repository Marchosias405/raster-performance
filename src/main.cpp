#include "raster.h"

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <time.h>
#include <vector>
#include <limits>

// A benchmark kernel takes an input raster and writes
// into an already allocated output raster.
using SmoothingKernel = void (*)(
    const Raster&,
    Raster&
);

// Convert two clock_gettime timestamps into milliseconds.
double elapsed_milliseconds(
    const timespec& start,
    const timespec& end
) {
    const double seconds =
        static_cast<double>(
            end.tv_sec - start.tv_sec
        );

    const double nanoseconds =
        static_cast<double>(
            end.tv_nsec - start.tv_nsec
        );

    return
        (seconds * 1000.0) +
        (nanoseconds / 1000000.0);
}

// Calculate the median timing.

// Median is useful because one unusually slow run has
// less influence than it would on an average.
double calculate_median(
    std::vector<double> times
) {
    std::sort(
        times.begin(),
        times.end()
    );

    const std::size_t middle =
        times.size() / 2;

    // Odd number of measurements.
    if (times.size() % 2 == 1) {
        return times[middle];
    }

    // Even number of measurements.
    return
        (times[middle - 1] + times[middle]) /
        2.0;
}

// Use the final output after timing.

// This gives us a simple correctness sanity value and
// ensures the produced output is actually observed.
double calculate_checksum(
    const Raster& raster
) {
    double sum = 0.0;

    for (float value : raster.values) {
        sum += value;
    }

    return sum;
}

// Convert a command-line argument to a positive size.

// Width, height, and repetitions must:
// - contain digits only
// - be greater than zero
// - fit inside std::size_t
std::size_t parse_positive_size(
    const char* text,
    const std::string& name
) {
    const std::string value_text(text);

    // Reject empty values and characters such as:
    // -, +, letters, decimal points, etc.
    if (value_text.empty()) {
        throw std::invalid_argument(
            name + " must be a positive integer"
        );
    }

    for (char character : value_text) {
        if (character < '0' || character > '9') {
            throw std::invalid_argument(
                name + " must be a positive integer"
            );
        }
    }

    try {
        const unsigned long long value =
            std::stoull(value_text);

        if (value == 0) {
            throw std::invalid_argument(
                name + " must be a positive integer"
            );
        }

        if (
            value >
            static_cast<unsigned long long>(
                std::numeric_limits<std::size_t>::max()
            )
        ) {
            throw std::invalid_argument(
                name + " is too large"
            );
        }

        return static_cast<std::size_t>(value);
    }
    catch (const std::out_of_range&) {
        throw std::invalid_argument(
            name + " is too large"
        );
    }
}

// Select which smoothing implementation will be timed.
SmoothingKernel select_kernel(
    const std::string& implementation
) {
    if (implementation == "row") {
        return smooth_row_major_into;
    }

    if (implementation == "column") {
        return smooth_column_major_into;
    }

    if (implementation == "simd") {
        return smooth_simd_avx_into;
    }

    throw std::invalid_argument(
        "implementation must be row, column, or simd"
    );
}

int main(
    int argc,
    char* argv[]
) {
    // Expected command:

    // ./build/raster-performance <implementation>
    //     <width> <height> <repetitions>

    // Example:

    // ./build/raster-performance row 1024 1024 7

    if (argc != 5) {
        std::cerr
            << "Usage: "
            << argv[0]
            << " <row|column|simd>"
            << " <width>"
            << " <height>"
            << " <repetitions>\n";

        return 1;
    }

    try {
        const std::string implementation =
            argv[1];

        const std::size_t width =
            parse_positive_size(
                argv[2],
                "width"
            );

        const std::size_t height =
            parse_positive_size(
                argv[3],
                "height"
            );

        const std::size_t repetitions =
            parse_positive_size(
                argv[4],
                "repetitions"
            );

        // Select the implementation before timing.
        const SmoothingKernel kernel =
            select_kernel(
                implementation
            );

        // Dataset generation is deliberately outside
        // the timed benchmark region.
        const Raster input =
            generate_synthetic_raster(
                width,
                height
            );

        // Output allocation and border initialization are
        // also deliberately outside the timed region.
        Raster output = input;

        std::vector<double> times;
        times.reserve(repetitions);

        // Repeat the same kernel several times.

        // The output does not need to be reallocated between
        // repetitions because every interior cell is overwritten
        // from the unchanged input raster.
        for (std::size_t repetition = 0;
             repetition < repetitions;
             ++repetition) {

            timespec start{};
            timespec end{};

            if (
                clock_gettime(
                    CLOCK_MONOTONIC,
                    &start
                ) != 0
            ) {
                throw std::runtime_error(
                    "clock_gettime failed at benchmark start"
                );
            }

            // --------- TIMED REGION ----------------
            kernel(
                input,
                output
            );
            // --------- END TIMED REGION --------------

            if (
                clock_gettime(
                    CLOCK_MONOTONIC,
                    &end
                ) != 0
            ) {
                throw std::runtime_error(
                    "clock_gettime failed at benchmark end"
                );
            }

            times.push_back(
                elapsed_milliseconds(
                    start,
                    end
                )
            );
        }

        // Checksum happens after all timing is complete.
        const double checksum =
            calculate_checksum(
                output
            );

        const double median =
            calculate_median(
                times
            );

        const auto minimum =
            std::min_element(
                times.begin(),
                times.end()
            );

        const auto maximum =
            std::max_element(
                times.begin(),
                times.end()
            );

        std::cout
            << std::fixed
            << std::setprecision(3);

        std::cout
            << "implementation: "
            << implementation
            << '\n';

        std::cout
            << "width: "
            << width
            << '\n';

        std::cout
            << "height: "
            << height
            << '\n';

        std::cout
            << "cells: "
            << input.values.size()
            << '\n';

        std::cout
            << "repetitions: "
            << repetitions
            << '\n';

        std::cout
            << "median_ms: "
            << median
            << '\n';

        std::cout
            << "min_ms: "
            << *minimum
            << '\n';

        std::cout
            << "max_ms: "
            << *maximum
            << '\n';

        std::cout
            << "checksum: "
            << checksum
            << '\n';
    }
    catch (const std::exception& error) {
        std::cerr
            << "Error: "
            << error.what()
            << '\n';

        return 1;
    }

    return 0;
}