# GIS-Style Raster Performance Analysis

CMPT 295 mini-project comparing compiler optimization, SIMD, memory locality/cache behaviour, and branch prediction using C++ raster-processing workloads.

## Build and Test

```bash
make clean
make
make test
```

Final benchmarks were run on CSIL (`asb9838nu-a05`, Intel i7-9700K, Ubuntu 24.04.4, g++ 13.3.0).

Examples:

```bash
taskset -c 0 ./build/raster-performance row 2048 2048 9
taskset -c 0 ./build/raster-performance column 2048 2048 9
taskset -c 0 ./build/raster-performance simd 2048 2048 9

make branch OPT="-O3 -fno-tree-vectorize"
taskset -c 0 ./build/branch-benchmark branch 2048 2048 50 shuffled 9
taskset -c 0 ./build/branch-benchmark branchless 2048 2048 50 shuffled 9
```

## Results

`results/` contains the evidence used in the report:

- Phase 9: compiler timing (`phase9_*`)
- Phase 10: vectorization/assembly (`phase10_*`)
- Phase 11: locality timing + `perf` (`phase11_*`)
- Phase 12: scalar/manual AVX/compiler SIMD timing (`phase12_*`)
- Phase 13: branch timing + `perf` + assembly (`phase13_*`)
- Phase 14: CSIL environment + Valgrind (`phase14_*`)
- Phase 15: final graphs (`phase15_*.png`)

`*_raw.csv` files contain recorded benchmark measurements from CSIL. `*_summary.csv` files contain the medians/comparisons used in the report. `.txt`/`.s` files preserve compiler, assembly, environment, or Valgrind evidence.

Graphs are generated from the saved summary CSVs with:

```bash
python3 scripts/plot_phase15.py
```
