# Phase 15 - Consolidated Performance Analysis

This file summarizes the verified experimental results from the raster-performance project.

The final performance experiments were run on the CSIL benchmark machine documented in `phase14_environment.txt`.

The project studies five main performance topics:

- compiler optimization,
- compiler auto-vectorization,
- memory locality,
- manual SIMD,
- branch prediction.

Raw measurements remain in the corresponding CSV, assembly, and environment files under `results/`.

---

## 1. Compiler Optimization

The row-major smoothing implementation was compiled using:

- `-O0`
- `-O2`
- `-O3`
- `-O3 -fno-tree-vectorize`

Each configuration used the same benchmark implementation and raster sizes.

### Largest raster: 2048 x 2048

| Build | Median time (ms) |
|---|---:|
| O0 | 87.535 |
| O2 | 8.216 |
| O3 | 2.003 |
| O3-novec | 8.369 |

### Main observations

For the 2048 x 2048 raster:

- `-O3` was about 43.7x faster than `-O0`.
- `-O3` was about 4.10x faster than `-O2`.
- `-O3` was about 4.18x faster than `-O3 -fno-tree-vectorize`.

The difference between `-O3` and `-O3 -fno-tree-vectorize` shows that compiler vectorization was a major contributor to the performance improvement.

However, the full `-O0` to `-O3` speedup must not be attributed only to SIMD because `-O3` also enables other compiler optimizations.

---

## 2. Compiler Auto-Vectorization Evidence

GCC's vectorization report showed that the inner row-major smoothing loop was automatically vectorized.

Relevant compiler evidence:

```text
src/raster.cpp:66:21: optimized: loop vectorized using 32 byte vectors
src/raster.cpp:66:21: optimized: loop versioned for vectorization because of possible aliasing
src/raster.cpp:66:21: optimized: loop vectorized using 16 byte vectors
```

The generated `-O3` assembly also contains packed floating-point AVX instructions such as:

```text
vbroadcastss
vmovups
vaddps
vdivps
```

The corresponding `-O3 -fno-tree-vectorize` version does not contain the same packed AVX loop.

### Conclusion

The compiler evidence and timing results together support the conclusion that GCC automatically vectorized the row-major smoothing loop and that this vectorization contributed substantially to the `-O3` performance improvement.

---

## 3. Memory Locality

The row-major and column-major smoothing implementations perform the same 3 x 3 averaging calculation.

The main difference is traversal order:

- row-major moves across adjacent columns first,
- column-major moves across rows first.

Because raster values are stored contiguously in row-major order, row-major traversal is expected to have better spatial locality.

The locality experiment used `-O3 -fno-tree-vectorize` to reduce the effect of automatic SIMD and make the traversal-order comparison easier to interpret.

### Timing results

| Raster size | Row-major (ms) | Column-major (ms) | Column slowdown |
|---|---:|---:|---:|
| 64 x 64 | 0.018 | 0.008 | 0.444x |
| 181 x 181 | 0.298 | 0.307 | 1.030x |
| 1254 x 1254 | 2.831 | 13.400 | 4.733x |
| 2048 x 2048 | 8.187 | 56.143 | 6.858x |

The smallest raster timings are extremely short and should not be used for strong conclusions. The important trend appears as the working set becomes larger.

### Cache evidence at 1254 x 1254

Row-major:

```text
L1 miss rate: 1.762%
L1 misses:    2,384,819
```

Column-major:

```text
L1 miss rate: 37.529%
L1 misses:    50,864,972
```

Column-major was 4.733x slower.

### Cache evidence at 2048 x 2048

Row-major:

```text
L1 miss rate: 1.999%
L1 misses:    7,215,674
LLC misses:   73,210
```

Column-major:

```text
L1 miss rate: 38.475%
L1 misses:    139,157,538
LLC misses:   18,053,003
```

Column-major was 6.858x slower.

### Conclusion

The performance gap increased greatly as the raster became larger.

At 2048 x 2048, column-major traversal:

- was about 6.86x slower,
- had roughly a 38.5% measured L1 data-cache miss rate,
- while row-major had roughly a 2.0% measured L1 data-cache miss rate.

This is consistent with row-major traversal benefiting from spatial locality in the contiguous row-major memory layout.

The raster sizes were selected relative to the measured cache hierarchy, but they should be described as approximate working-set comparisons rather than exact cache fits.

The `perf stat` cache counters measure the complete process rather than only the timed kernel, so they are supporting evidence rather than exact kernel-only cache counts.

---

## 4. Manual SIMD

The project contains a manual 256-bit AVX smoothing implementation.

Eight single-precision floating-point values can be processed in one 256-bit AVX vector.

The SIMD experiment compared:

1. row-major scalar code compiled with `-O3 -fno-tree-vectorize`,
2. manual AVX compiled with the same optimization configuration,
3. row-major code compiled with normal `-O3` auto-vectorization.

### Results

| Raster size | Scalar no-vector (ms) | Manual AVX (ms) | Auto-vectorized (ms) |
|---|---:|---:|---:|
| 64 x 64 | 0.036 | 0.013 | 0.010 |
| 181 x 181 | 0.265 | 0.069 | 0.055 |
| 1254 x 1254 | 2.750 | 0.782 | 0.807 |
| 2048 x 2048 | 8.477 | 2.155 | 1.940 |

### Manual AVX speedup

At 1254 x 1254, manual AVX was about 3.52x faster than the non-vectorized scalar baseline.

At 2048 x 2048, manual AVX was about 3.93x faster than the non-vectorized scalar baseline.

At 2048 x 2048, GCC's auto-vectorized version ran in 1.940 ms compared with 2.155 ms for the manual AVX implementation, so the auto-vectorized version was about 1.11x faster.

At 1254 x 1254, manual AVX was slightly faster:

```text
manual AVX:      0.782 ms
auto-vectorized: 0.807 ms
```

### Conclusion

Manual AVX produced a large improvement over the non-vectorized scalar baseline.

However, handwritten SIMD did not universally outperform compiler-generated SIMD. GCC's automatic vectorization was competitive and was slightly faster for the largest tested raster.

---

## 5. Branch Prediction

The branch-prediction experiment compared:

- an explicit branch implementation,
- a source-level branchless implementation.

The controlled datasets used:

- 5% passing values,
- 50% passing values,
- 95% passing values,

with two orderings:

- grouped,
- shuffled.

The raster size was 2048 x 2048 and the classification threshold was 0.5.

Generated values were:

```text
0.25 -> fail
0.75 -> pass
```

The shuffled ordering used a fixed deterministic seed.

---

## 6. Branch Machine-Code Verification

Before collecting branch-prediction measurements, the generated assembly was inspected using:

```text
-O3 -fno-tree-vectorize -mavx
```

The explicit branch implementation contains:

```text
vcomiss
ja
```

The `ja` instruction is a data-dependent conditional branch.

The branchless implementation contains:

```text
vcomiss
seta
```

The `seta` instruction converts the comparison result to 0 or 1 without a data-dependent conditional jump.

Both implementations still contain loop-control branches, which is expected.

This assembly evidence confirms that the benchmark compares a genuine data-dependent branch against a branchless classification operation.

---

## 7. Branch-Prediction Results

### Grouped datasets

| Pass rate | Branch time (ms) | Branch miss rate | Branchless time (ms) |
|---|---:|---:|---:|
| 5% | 4.118 | 0.021% | 3.162 |
| 50% | 4.398 | 0.021% | 3.149 |
| 95% | 4.838 | 0.021% | 3.181 |

All three grouped distributions produced very low measured branch-miss rates.

This demonstrates that a 50/50 outcome distribution does not automatically produce poor branch prediction when the values occur in long predictable groups.

### Shuffled datasets

| Pass rate | Branch time (ms) | Branch miss rate | Branchless time (ms) | Branchless speedup |
|---|---:|---:|---:|---:|
| 5% | 5.180 | 2.162% | 3.067 | 1.689x |
| 50% | 17.490 | 20.356% | 3.075 | 5.688x |
| 95% | 6.115 | 2.110% | 3.024 | 2.022x |

### Most important branch result

The shuffled 50% dataset produced:

```text
branch:
17.490 ms
20.356% measured branch-miss rate

branchless:
3.075 ms
0.051% measured branch-miss rate
```

The branchless version was approximately 5.688x faster than the explicit branch for this dataset.

### Conclusion

Data ordering had a major effect on branch predictability.

The grouped 50/50 dataset was easy for the branch predictor because it consisted of long runs of the same outcome.

The shuffled 50/50 dataset was much harder to predict and produced:

- the highest measured branch-miss rate,
- the slowest explicit-branch running time,
- and the largest benefit from branchless code.

The branchless implementation stayed close to 3 ms across all tested distributions because its main classification decision does not use a data-dependent conditional jump.

`perf stat` measured branch events for the complete process. Each perf invocation ran the classification kernel 20 times so that classification work dominated the process, but the counters should still be described as process-level measurements.

---

## 8. Main Findings

| Topic | Key comparison | Main result |
|---|---|---|
| Compiler optimization | O0 vs O3 at 2048 x 2048 | O3 about 43.7x faster |
| Auto-vectorization | O3-novec vs O3 at 2048 x 2048 | O3 about 4.18x faster |
| Memory locality | row vs column at 2048 x 2048 | column about 6.86x slower |
| Manual SIMD | scalar vs manual AVX at 2048 x 2048 | manual AVX about 3.93x faster |
| Branch prediction | shuffled 50% branch vs branchless | branchless about 5.69x faster |

---

## 9. Overall Conclusion

The experiments show that GIS-style raster-processing performance depends strongly on how software interacts with the underlying processor.

Compiler optimization greatly improved smoothing performance, and GCC's automatic SIMD vectorization was a major contributor to the optimized result.

Manual AVX also produced a substantial speedup over scalar non-vectorized code, although the compiler-generated SIMD implementation was competitive and sometimes faster.

Memory traversal order became increasingly important as raster size increased. Row-major traversal matched the raster's contiguous memory layout and produced substantially fewer measured cache misses than column-major traversal for large rasters.

Branch-prediction performance depended strongly on data ordering. Grouped outcomes were highly predictable even for a 50/50 distribution, while a shuffled 50/50 dataset produced a large increase in branch misses and running time.

Overall, the project demonstrates that compiler optimization, SIMD, cache locality, and branch predictability can each have large effects on the performance of the same underlying raster-processing workload.

---

## 10. Recommended Final Report Evidence

Because the final report is limited to approximately 2-3 pages, the raw measurements should remain in the repository while the report uses only the strongest evidence.

Recommended summary table:

| Topic | Key comparison | Main result |
|---|---|---|
| Compiler optimization | O0 vs O3 at 2048 x 2048 | O3 about 43.7x faster |
| Auto-vectorization | O3-novec vs O3 at 2048 x 2048 | O3 about 4.18x faster |
| Memory locality | row vs column at 2048 x 2048 | column about 6.86x slower |
| Manual SIMD | scalar vs manual AVX at 2048 x 2048 | manual AVX about 3.93x faster |
| Branch prediction | shuffled 50% branch vs branchless | branchless about 5.69x faster |

Recommended figures:

1. Row-major versus column-major smoothing time as raster size increases.
2. Branch and branchless classification performance for grouped and shuffled 5%, 50%, and 95% distributions.

Compiler and SIMD results can be shown in a compact table if report space is limited.
