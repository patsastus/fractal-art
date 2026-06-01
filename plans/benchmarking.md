# Benchmarking Plan

## Goal

Measure the rendering performance of the **original C** implementation against the
**C++ refactor**, and quantify the impact of template dispatch vs. function pointer
dispatch. The benchmark should produce reproducible numbers that isolate the
**compute cost** from windowing/display overhead.

---

## What to Measure

| Metric | Why |
|---|---|
| **Frame render time** (ms) | The wall-clock time for one full `draw_fractal` call |
| **Throughput** (Mpixels/sec) | Resolution-independent metric: `(width × height) / render_time` |
| **Iterations/sec** (Giters/sec) | Normalizes across different `max_iters` settings |

All three are derived from the same timing — just different denominators.

---

## Benchmark Scenarios

Run each scenario for **both** C and C++ versions:

| Scenario | Fractal | Resolution | Max Iters | Purpose |
|---|---|---|---|---|
| **S1: Original** | Mandelbrot | 500×500 | 100 | Baseline matching original settings |
| **S2: HD** | Mandelbrot | 1920×1080 | 100 | Resolution scaling |
| **S3: Deep** | Mandelbrot | 500×500 | 1000 | Iteration scaling |
| **S4: HD+Deep** | Mandelbrot | 1920×1080 | 1000 | Combined stress test |
| **S5: Julia** | Julia (-0.4, 0.6) | 1920×1080 | 1000 | Different fractal, same workload class |
| **S6: Newton** | Newton | 1920×1080 | 100 | Heavy per-iteration work (complex division) |

Each scenario runs **100 frames** at the same zoom level (default view). Report
**median**, **p5**, and **p95** render times.

---

## Implementation Strategy

### 1. Headless Benchmark Mode

Both versions need a `--benchmark` CLI flag that:
- Skips MLX window creation entirely (no GPU, no display dependency)
- Allocates a raw pixel buffer (`uint32_t[width * height]`)
- Runs `draw_fractal` N times against that buffer
- Prints per-frame timing results to stdout as CSV

This ensures we're measuring **pure CPU compute**, not display/vsync/event overhead.

### 2. Instrumentation for the C Version

Add a new file `benchmark_c.c` in the `src/benchmark/` directory. This avoids modifying the original
source files.

```c
// src/benchmark/benchmark_c.c
#include <fractol.h>
#include <time.h>
#include <stdio.h>

// We need draw_fractal's inner logic without MLX.
// Option A: extract the compute loop into a separate function.
// Option B: write a minimal benchmark loop that mirrors draw_fractal.

static void bench_draw(uint32_t* buffer, uint32_t width, uint32_t height,
                        t_fractal* f, t_anchor* a,
                        uint32_t (*iterator)(t_complex*, t_fractal*)) {
    int32_t x = 0;
    t_complex temp[2];

    pixel_to_complex(&temp[0], 0, 0, a);
    set_complex(&temp[1], temp[0].re, temp[0].im);
    while ((uint32_t)x < width) {
        int32_t y = 0;
        while ((uint32_t)y < height) {
            uint32_t i = iterator(&temp[1], f);
            buffer[y * width + x] = f->colors[i];
            set_complex(&temp[1], temp[1].re, temp[1].im - a->px_step);
            ++y;
        }
        set_complex(&temp[1], temp[1].re + a->px_step, temp[0].im);
        ++x;
    }
}

static double time_ms(struct timespec* start, struct timespec* end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 +
           (end->tv_nsec - start->tv_nsec) / 1e6;
}

int main(int argc, char** argv) {
    // Parse: ./benchmark_c <fractal> <width> <height> <max_iters> <n_frames>
    // ... (parse arguments, init fractal + anchor + colors)

    uint32_t* buffer = malloc(sizeof(uint32_t) * width * height);
    struct timespec t0, t1;

    // Warmup: 5 frames
    for (int i = 0; i < 5; i++)
        bench_draw(buffer, width, height, &f, &a, iterator);

    // Timed runs
    printf("frame,time_ms\n");
    for (int i = 0; i < n_frames; i++) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        bench_draw(buffer, width, height, &f, &a, iterator);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("%d,%.3f\n", i, time_ms(&t0, &t1));
    }

    free(buffer);
    return 0;
}
```

**Key point:** `bench_draw` is a copy of the inner loop from `draw_fractal` in
`visuals_bonus.c`, but writes to a plain buffer instead of calling `mlx_put_pixel`.
This eliminates any MLX overhead from the measurement.

*Note on initialization:* The C version relies on `init_mandelbrot`, `init_julia`, etc. These allocate the color palette via `make_colors` but fortunately do not depend on MLX directly. Ensure you do not call `init_all` (which creates the window) in the benchmark harness.

### 3. Instrumentation for the C++ Version

Create a standalone `src/benchmark/benchmark_cpp.cpp`:

```cpp
// src/benchmark/benchmark_cpp.cpp
#include "complex.hpp"
#include "fractal.hpp"
#include "utils.hpp"
#include <chrono>
#include <iostream>
#include <vector>
#include <memory>

template<typename F>
void bench_draw(uint32_t* buffer, uint32_t width, uint32_t height,
                const F& fractal, const Anchor& anchor) {
    Complex origin = Complex::from_pixel(0, 0, anchor);
    Complex col_start = origin;
    for (uint32_t x = 0; x < width; ++x) {
        Complex z = col_start;
        for (uint32_t y = 0; y < height; ++y) {
            uint32_t i = fractal.iterate(z);  // inlined
            buffer[y * width + x] = fractal.colors[i];
            z.im -= anchor.px_step;
        }
        col_start.re += anchor.px_step;
    }
}

void run_benchmark(Fractal& fractal, uint32_t width, uint32_t height,
                   int n_frames, const Anchor& anchor) {
    std::vector<uint32_t> buffer(width * height);

    // Warmup
    for (int i = 0; i < 5; i++) {
        if (auto* m = dynamic_cast<Mandelbrot*>(&fractal))
            bench_draw(buffer.data(), width, height, *m, anchor);
        else if (auto* j = dynamic_cast<Julia*>(&fractal))
            bench_draw(buffer.data(), width, height, *j, anchor);
        else if (auto* n = dynamic_cast<Newton*>(&fractal))
            bench_draw(buffer.data(), width, height, *n, anchor);
    }

    // Timed runs
    std::cout << "frame,time_ms\n";
    for (int i = 0; i < n_frames; i++) {
        auto t0 = std::chrono::high_resolution_clock::now();

        if (auto* m = dynamic_cast<Mandelbrot*>(&fractal))
            bench_draw(buffer.data(), width, height, *m, anchor);
        else if (auto* j = dynamic_cast<Julia*>(&fractal))
            bench_draw(buffer.data(), width, height, *j, anchor);
        else if (auto* n = dynamic_cast<Newton*>(&fractal))
            bench_draw(buffer.data(), width, height, *n, anchor);

        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        std::cout << i << "," << ms << "\n";
    }
}

int main(int argc, char** argv) {
    // Parse: ./benchmark_cpp <fractal> <width> <height> <max_iters> <n_frames>
    // ... (create fractal, init anchor + colors, call run_benchmark)
}
```

### 4. Build Integration

```cmake
# src/benchmark/CMakeLists.txt (or added to root CMakeLists.txt)

# C benchmark — compile the C sources + benchmark harness
add_executable(benchmark_c
    src/benchmark/benchmark_c.c
    src/c/complex.c
    src/c/mandelbrot.c
    src/c/julia.c
    src/c/newton.c
    src/c/newton_utils.c
    src/c/utils.c
    src/c/init.c
)
target_include_directories(benchmark_c PRIVATE include/)
target_compile_options(benchmark_c PRIVATE -O3 -ffast-math)
target_link_libraries(benchmark_c PRIVATE m)

# C++ benchmark
add_executable(benchmark_cpp
    src/benchmark/benchmark_cpp.cpp
    src/cpp/complex.cpp
    src/cpp/fractal.cpp
    src/cpp/utils.cpp
)
target_include_directories(benchmark_cpp PRIVATE src/cpp/)
target_compile_options(benchmark_cpp PRIVATE -O3 -ffast-math)
target_link_libraries(benchmark_cpp PRIVATE m)
```

> **Note:** The C benchmark links against `src/c/*.c` directly but does NOT link
> MLX42 — it only uses the pure-compute functions. The `init.c` inclusion
> is needed for `make_colors` and `make_colors_newton`, but `init_all` (which
> needs MLX) would need to be either stubbed or we extract just the palette
> init into a separate function.

### 5. Output Validation (Sanity Check)

Before trusting any performance numbers, we must guarantee that the C++ refactor produces the exact same computational workload as the original C version. If the C++ version skips iterations due to a subtle logic bug, the speedup numbers will be fraudulent.

- In the harness (for both C and C++), compute a fast hash (e.g., a simple XOR sum or DJB2 hash) of the `buffer` after the first rendered frame.
- Print this checksum to stdout alongside the timing headers.
- The runner/analysis script must assert that the checksum from the C version matches the checksum from the C++ version for each scenario. If they differ, the benchmark run must be flagged as invalid.

---

## Runner Script

A shell script to run all scenarios and collect results:

```bash
#!/bin/bash
# src/benchmark/run_benchmarks.sh

FRAMES=100
OUTDIR=src/benchmark/results/$(date +%Y%m%d_%H%M%S)
mkdir -p "$OUTDIR"

scenarios=(
    "m 500 500 100"
    "m 1920 1080 100"
    "m 500 500 1000"
    "m 1920 1080 1000"
    "j 1920 1080 1000"     # Julia uses default c=-0.4+0.6i
    "n 1920 1080 100"
)
names=(
    "S1_original"
    "S2_hd"
    "S3_deep"
    "S4_hd_deep"
    "S5_julia"
    "S6_newton"
)

for i in "${!scenarios[@]}"; do
    echo "=== ${names[$i]} ==="
    ./build/benchmark_c  ${scenarios[$i]} $FRAMES > "$OUTDIR/${names[$i]}_c.csv"
    ./build/benchmark_cpp ${scenarios[$i]} $FRAMES > "$OUTDIR/${names[$i]}_cpp.csv"
done

echo "Results saved to $OUTDIR"
```

---

## Analysis

After collecting CSV files, compute summary statistics with a small script
(`src/benchmark/analyze.py` or even `awk`):

```
Scenario          | C median (ms) | C++ median (ms) | Speedup | C++ Mpx/s
------------------|---------------|------------------|---------|----------
S1: 500² × 100   |               |                  |         |
S2: 1080p × 100  |               |                  |         |
S3: 500² × 1000  |               |                  |         |
S4: 1080p × 1000 |               |                  |         |
S5: Julia 1080p  |               |                  |         |
S6: Newton 1080p |               |                  |         |
```

**Expected results:**
- S1 should show a modest speedup (small workload, overhead dominates)
- S3 and S4 should show the **largest speedup**, because the inner loop
  (which benefits most from inlining/SIMD) dominates the total time
- S6 (Newton) may show less relative improvement because complex division
  is harder for the compiler to vectorize

---

## Pitfalls to Avoid

| Pitfall | Mitigation |
|---|---|
| **Compiler optimizing away the buffer** | Write a `volatile` read of one pixel after the loop, or use `benchmark::DoNotOptimize` |
| **Turbo boost / thermal throttling** | Use the warmup phase; run on a quiet machine; report median not mean |
| **Different optimization levels** | Both must compile with identical `-O3 -ffast-math`. Ensure the C and C++ compilers are from the same family and version (e.g., GCC 13 for both). |
| **Cache effects** | Both versions use the same buffer layout (`uint32_t[w*h]`), so cache behavior should be comparable |
| **`mlx_put_pixel` overhead** | Benchmark harness writes to a raw buffer, not MLX, so this is eliminated from both versions |
| **Silent Correctness Failures**| Implement the **Output Validation** (checksum) step. Never trust a benchmark where the outputs aren't bit-for-bit identical. |

---

## Implementation Checklist

- [ ] Write `src/benchmark/benchmark_c.c` (headless C benchmark harness)
- [ ] Write `src/benchmark/benchmark_cpp.cpp` (headless C++ benchmark harness)
- [ ] Extract `make_colors` / `init_anchor` from `src/c/init.c` so they can be
      called without MLX (may need a small refactor or stub)
- [ ] Add benchmark targets to `CMakeLists.txt`
- [ ] Write `src/benchmark/run_benchmarks.sh` runner script
- [ ] Write `src/benchmark/analyze.py` (or a simple awk one-liner) for summary stats
- [ ] Run benchmarks on a consistent machine
- [ ] Record results in a `src/benchmark/results/` directory
