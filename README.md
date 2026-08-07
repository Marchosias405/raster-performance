# Performance Analysis of GIS-Style Raster Processing

This project uses synthetic GIS-style raster processing workloads to study CMPT 295 computer-systems concepts in C++ on x86-64.

The project focuses on:

- compiler optimization
- generated assembly
- compiler auto-vectorization
- manual AVX SIMD
- memory locality and cache behavior
- branch prediction
- performance measurement
- correctness and memory verification

The goal is not to build a complete GIS application. Raster processing is used as a scalable workload for systems-performance experiments.

## Workloads

The project uses two main workloads:

1. 3x3 raster smoothing
2. threshold classification

Raster values are stored in a contiguous row-major `std::vector<float>`.

For smoothing, border cells remain unchanged and only interior cells are replaced by the average of their 3x3 neighborhood.

Implemented smoothing versions:

- row-major scalar
- column-major scalar
- manual 256-bit AVX SIMD

Implemented classification versions:

- explicit branch
- branchless comparison

## Synthetic Data

No external GIS dataset is used.

The general raster generator is deterministic:

```text
value = (index % 1000) / 1000.0
```

Threshold-classification datasets use:

```text
0.25 -> fail
0.75 -> pass
threshold = 0.5
```

Controlled pass rates:

- 5%
- 50%
- 95%

Each threshold dataset can be grouped or deterministically shuffled.

Data generation and output allocation are performed outside the timed kernels.

## Benchmark Environment

Final performance experiments were run on the same CSIL machine:

```text
Host:         asb9838nu-a05
OS:           Ubuntu 24.04.4 LTS
Architecture: x86-64
CPU:          Intel Core i7-9700K @ 3.60 GHz
Logical CPUs: 8
Compiler:     g++ 13.3.0
perf:         7.0.12
Valgrind:     3.22.0
Cache line:   64 bytes
SIMD support: AVX, AVX2, FMA
```

Cache hierarchy:

```text
L1 data:        32 KiB per core
L1 instruction: 32 KiB per core
L2:             256 KiB per core
L3:             12 MiB shared
```

Final benchmark runs were pinned to CPU 0 with `taskset`.

## Key Results

### Compiler optimization

For a 2048 x 2048 row-major smoothing raster:

| Build | Median time |
|---|---:|
| `-O0` | 87.535 ms |
| `-O2` | 8.216 ms |
| `-O3` | 2.003 ms |
| `-O3 -fno-tree-vectorize` | 8.369 ms |

`-O3` was about:

- 43.7x faster than `-O0`
- 4.18x faster than `-O3 -fno-tree-vectorize`

GCC also reported that the row-major inner loop was vectorized using 32-byte vectors.

### Memory locality

For 2048 x 2048 smoothing:

| Traversal | Median time | L1 miss rate |
|---|---:|---:|
| Row-major | 8.187 ms | 1.999% |
| Column-major | 56.143 ms | 38.475% |

Column-major traversal was about 6.86x slower.

This is consistent with row-major traversal benefiting from spatial locality in the row-major memory layout.

### Manual SIMD

For 2048 x 2048 smoothing:

| Version | Median time |
|---|---:|
| Scalar, no auto-vectorization | 8.477 ms |
| Manual AVX | 2.155 ms |
| GCC auto-vectorized | 1.940 ms |

Manual AVX was about 3.93x faster than the non-vectorized scalar baseline.

The compiler-generated SIMD version was slightly faster than the manual AVX version for the largest raster.

### Branch prediction

The branch experiment used a 2048 x 2048 raster with 5%, 50%, and 95% passing values in grouped and shuffled order.

The strongest result was the shuffled 50% dataset:

| Version | Median time | Branch miss rate |
|---|---:|---:|
| Explicit branch | 17.490 ms | 20.356% |
| Branchless | 3.075 ms | 0.051% |

The branchless implementation was about 5.69x faster.

Generated assembly confirmed the intended machine-code difference:

```text
explicit branch:
vcomiss
ja

branchless:
vcomiss
seta
```

## Correctness

The test suite checks:

- deterministic raster generation
- hand-calculated smoothing cases
- row-major and column-major equivalence
- border preservation
- SIMD output against scalar output
- SIMD tail cases
- threshold behavior
- 5%, 50%, and 95% datasets
- grouped and shuffled classification
- benchmark-oriented `*_into()` functions

Run:

```bash
make test
```

Final Valgrind verification reported:

```text
in use at exit: 0 bytes in 0 blocks
All heap blocks were freed -- no leaks are possible
ERROR SUMMARY: 0 errors
```

## Build and Run

Build the smoothing benchmark:

```bash
make
```

Build with a chosen optimization level:

```bash
make clean
make OPT=-O3
```

Run the smoothing benchmark:

```bash
./build/raster-performance row 2048 2048 9
./build/raster-performance column 2048 2048 9
./build/raster-performance simd 2048 2048 9
```

Build the branch benchmark:

```bash
make clean
make branch OPT="-O3 -fno-tree-vectorize"
```

Run it:

```bash
./build/branch-benchmark branch 2048 2048 50 shuffled 9
./build/branch-benchmark branchless 2048 2048 50 shuffled 9
```

## Results

Raw and summarized experiment data is stored under:

```text
results/
```

This includes:

- compiler optimization measurements
- vectorization reports and assembly
- memory-locality timing and `perf` counters
- SIMD timing results
- branch-prediction timing and `perf` counters
- benchmark environment details
- Valgrind output
- final analysis and figures

The main figures are:

- `results/phase15_locality.png`
- `results/phase15_branch.png`

The plotting script is:

```text
scripts/plot_phase15.py
```

## Main Takeaway

The project shows that performance of the same raster-processing workload can change substantially depending on:

- compiler optimization
- SIMD/vectorization
- memory traversal order
- cache locality
- branch predictability

The raw evidence and generated assembly are preserved in the repository so the conclusions can be traced back to the actual measurements.
