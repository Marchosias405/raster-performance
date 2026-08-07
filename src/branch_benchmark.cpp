#include "raster.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

// Function-pointer type for the two classification kernels.
//
// Both kernels:
// - read the same Raster
// - use the same threshold
// - write into an already allocated output vector
using Classifier = void (*)(
    const Raster&,
    float,
    std::vector<std::uint8_t>&
);

// Convert a command-line argument to a positive size_t.
//
// We reject:
// - empty strings
// - negative values
// - non-numeric values
// - zero
// - values too large for size_t
std::size_t parse_positive_size(
    const std::string& text,
    const std::string& name
) {
    if (text.empty()) {
        throw std::invalid_argument(
            name + " must not be empty"
        );
    }

    for (unsigned char character : text) {
        if (!std::isdigit(character)) {
            throw std::invalid_argument(
                name + " must contain only digits"
            );
        }
    }

    std::size_t position = 0;

    const unsigned long long value =
        std::stoull(
            text,
            &position
        );

    if (position != text.size()) {
        throw std::invalid_argument(
            name + " must be a valid integer"
        );
    }

    if (value == 0) {
        throw std::invalid_argument(
            name + " must be greater than zero"
        );
    }

    if (
        value >
        static_cast<unsigned long long>(
            std::numeric_limits<std::size_t>::max()
        )
    ) {
        throw std::out_of_range(
            name + " is too large"
        );
    }

    return static_cast<std::size_t>(value);
}

// Parse a passing percentage.
//
// Valid range:
// 0 through 100.
//
// The actual experiment will use:
// 5%, 50%, and 95%.
std::size_t parse_percentage(
    const std::string& text
) {
    if (text.empty()) {
        throw std::invalid_argument(
            "pass_percent must not be empty"
        );
    }

    for (unsigned char character : text) {
        if (!std::isdigit(character)) {
            throw std::invalid_argument(
                "pass_percent must contain only digits"
            );
        }
    }

    std::size_t position = 0;

    const unsigned long long value =
        std::stoull(
            text,
            &position
        );

    if (position != text.size()) {
        throw std::invalid_argument(
            "pass_percent must be a valid integer"
        );
    }

    if (value > 100) {
        throw std::invalid_argument(
            "pass_percent must be between 0 and 100"
        );
    }

    return static_cast<std::size_t>(value);
}

// Calculate elapsed milliseconds using CLOCK_MONOTONIC.
//
// CLOCK_MONOTONIC is appropriate for elapsed-time measurements
// because it does not move backwards when wall-clock time changes.
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
        seconds * 1000.0 +
        nanoseconds / 1000000.0;
}

// Return the median of the recorded running times.
//
// For an odd number of measurements:
// use the middle value.
//
// For an even number:
// average the two middle values.
double calculate_median(
    std::vector<double> values
) {
    std::sort(
        values.begin(),
        values.end()
    );

    const std::size_t middle =
        values.size() / 2;

    if (values.size() % 2 == 1) {
        return values[middle];
    }

    return
        (
            values[middle - 1] +
            values[middle]
        ) / 2.0;
}

// Count how many output cells were classified as true.
//
// This happens after timing so it does not become part
// of the measured classification kernel.
std::size_t count_true(
    const std::vector<std::uint8_t>& output
) {
    return static_cast<std::size_t>(
        std::count(
            output.begin(),
            output.end(),
            static_cast<std::uint8_t>(1)
        )
    );
}

int main(
    int argc,
    char* argv[]
) {
    // Required format:
    //
    // ./branch-benchmark
    //     <branch|branchless>
    //     <width>
    //     <height>
    //     <pass_percent>
    //     <grouped|shuffled>
    //     <repetitions>
    if (argc != 7) {
        std::cerr
            << "Usage: "
            << argv[0]
            << " <branch|branchless>"
            << " <width>"
            << " <height>"
            << " <pass_percent>"
            << " <grouped|shuffled>"
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

        const std::size_t pass_percent =
            parse_percentage(
                argv[4]
            );

        const std::string order =
            argv[5];

        const std::size_t repetitions =
            parse_positive_size(
                argv[6],
                "repetitions"
            );

        // Avoid overflow in width * height.
        if (
            width >
            std::numeric_limits<std::size_t>::max() /
            height
        ) {
            throw std::overflow_error(
                "width * height is too large"
            );
        }

        const std::size_t total_cells =
            width * height;

        // generate_threshold_raster() calculates:
        //
        // total_cells * pass_percent / 100
        //
        // so also reject values that could overflow
        // during that multiplication.
        if (
            total_cells >
            std::numeric_limits<std::size_t>::max() /
            100
        ) {
            throw std::overflow_error(
                "raster is too large"
            );
        }

        Classifier classifier = nullptr;

        if (implementation == "branch") {
            classifier =
                classify_branch_into;
        } else if (
            implementation == "branchless"
        ) {
            classifier =
                classify_branchless_into;
        } else {
            throw std::invalid_argument(
                "implementation must be "
                "'branch' or 'branchless'"
            );
        }

        bool shuffled = false;

        if (order == "grouped") {
            shuffled = false;
        } else if (order == "shuffled") {
            shuffled = true;
        } else {
            throw std::invalid_argument(
                "order must be "
                "'grouped' or 'shuffled'"
            );
        }

        // Fixed experimental constants.
        //
        // threshold_raster uses:
        // 0.25f = fail
        // 0.75f = pass
        //
        // Therefore threshold 0.5f cleanly separates them.
        constexpr float threshold = 0.5f;

        // Fixed shuffle seed makes the shuffled dataset
        // reproducible across repeated runs.
        constexpr std::uint32_t seed = 12345;

        // Generate the dataset BEFORE timing.
        //
        // This prevents data generation and shuffling from
        // contaminating classification measurements.
        const Raster input =
            generate_threshold_raster(
                width,
                height,
                pass_percent,
                shuffled,
                seed
            );

        // Allocate output BEFORE timing.
        //
        // Both kernels overwrite every output element.
        std::vector<std::uint8_t> output(
            input.values.size(),
            0
        );

        std::vector<double> times;
        times.reserve(repetitions);

        // Time only the classification kernel.
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
                    "clock_gettime failed"
                );
            }

            classifier(
                input,
                threshold,
                output
            );

            if (
                clock_gettime(
                    CLOCK_MONOTONIC,
                    &end
                ) != 0
            ) {
                throw std::runtime_error(
                    "clock_gettime failed"
                );
            }

            times.push_back(
                elapsed_milliseconds(
                    start,
                    end
                )
            );
        }

        // Validate the final output after timing.
        const std::size_t true_count =
            count_true(output);

        const std::size_t expected_true_count =
            (
                total_cells *
                pass_percent
            ) / 100;

        if (
            true_count !=
            expected_true_count
        ) {
            std::cerr
                << "Error: classification result "
                << "does not match expected count\n";

            return 2;
        }

        const double median_ms =
            calculate_median(times);

        const double min_ms =
            *std::min_element(
                times.begin(),
                times.end()
            );

        const double max_ms =
            *std::max_element(
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
            << "order: "
            << order
            << '\n';

        std::cout
            << "pass_percent: "
            << pass_percent
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
            << total_cells
            << '\n';

        std::cout
            << "repetitions: "
            << repetitions
            << '\n';

        std::cout
            << "median_ms: "
            << median_ms
            << '\n';

        std::cout
            << "min_ms: "
            << min_ms
            << '\n';

        std::cout
            << "max_ms: "
            << max_ms
            << '\n';

        std::cout
            << "true_count: "
            << true_count
            << '\n';

        std::cout
            << "expected_true_count: "
            << expected_true_count
            << '\n';

        return 0;
    } catch (const std::exception& error) {
        std::cerr
            << "Error: "
            << error.what()
            << '\n';

        return 1;
    }
}