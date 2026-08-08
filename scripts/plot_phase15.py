import csv
from pathlib import Path

import matplotlib.pyplot as plt


RESULTS_DIR = Path("results")


# -------------------------
# Graph 1: Memory locality

# Source:
# results/phase11_locality_summary.csv

# Compare row-major and column-major median kernel time across the four verified raster sizes.
# ---------------------------------------------------
def plot_locality():
    input_file = RESULTS_DIR / "phase11_locality_summary.csv"
    output_file = RESULTS_DIR / "phase15_locality.png"

    row_times = {}
    column_times = {}

    with input_file.open(newline="") as file:
        reader = csv.DictReader(file)

        for record in reader:
            size = (
                int(record["width"]),
                int(record["height"]),
            )

            median_ms = float(record["median_ms"])

            if record["implementation"] == "row":
                row_times[size] = median_ms

            elif record["implementation"] == "column":
                column_times[size] = median_ms

    sizes = sorted(row_times)

    labels = [
        f"{width}x{height}"
        for width, height in sizes
    ]

    row_values = [
        row_times[size]
        for size in sizes
    ]

    column_values = [
        column_times[size]
        for size in sizes
    ]

    x = list(range(len(sizes)))

    plt.figure(figsize=(8, 5))

    plt.plot(
        x,
        row_values,
        marker="o",
        label="Row-major"
    )

    plt.plot(
        x,
        column_values,
        marker="o",
        label="Column-major"
    )

    plt.xticks(x, labels)

    plt.xlabel("Raster size")
    plt.ylabel("Median kernel time (ms)")
    plt.title("Memory Locality: Row-major vs Column-major")
    plt.legend()
    plt.grid(axis="y", alpha=0.25)
    plt.tight_layout()

    plt.savefig(
        output_file,
        dpi=160
    )

    plt.close()

    print(f"Wrote {output_file}")


# ---------------------------------------
# Graph 2: Branch prediction

# Source:
# results/phase13_branch_summary.csv

# Show branch and branchless median kernel times for:

# grouped 5%, 50%, 95%
# shuffled 5%, 50%, 95%

# This makes the shuffled 50% branch slowdown easy to see.
# --------------------------------------------------
def plot_branch_prediction():
    input_file = RESULTS_DIR / "phase13_branch_summary.csv"
    output_file = RESULTS_DIR / "phase15_branch.png"

    times = {}

    with input_file.open(newline="") as file:
        reader = csv.DictReader(file)

        for record in reader:
            key = (
                record["implementation"],
                record["order"],
                int(record["pass_percent"]),
            )

            times[key] = float(
                record["median_ms"]
            )

    configurations = [
        ("grouped", 5),
        ("grouped", 50),
        ("grouped", 95),
        ("shuffled", 5),
        ("shuffled", 50),
        ("shuffled", 95),
    ]

    labels = [
        "Grouped\n5%",
        "Grouped\n50%",
        "Grouped\n95%",
        "Shuffled\n5%",
        "Shuffled\n50%",
        "Shuffled\n95%",
    ]

    branch_values = [
        times[
            ("branch", order, percent)
        ]
        for order, percent in configurations
    ]

    branchless_values = [
        times[
            ("branchless", order, percent)
        ]
        for order, percent in configurations
    ]

    x = list(range(len(configurations)))

    plt.figure(figsize=(9, 5))

    plt.plot(
        x,
        branch_values,
        marker="o",
        label="Explicit branch"
    )

    plt.plot(
        x,
        branchless_values,
        marker="o",
        label="Branchless"
    )

    plt.xticks(x, labels)

    plt.xlabel("Dataset distribution")
    plt.ylabel("Median kernel time (ms)")
    plt.title("Branch Prediction: Data Ordering and Performance")
    plt.legend()
    plt.grid(axis="y", alpha=0.25)
    plt.tight_layout()

    plt.savefig(
        output_file,
        dpi=160
    )

    plt.close()

    print(f"Wrote {output_file}")


def main():
    plot_locality()
    plot_branch_prediction()


if __name__ == "__main__":
    main()