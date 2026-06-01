#!/bin/bash
# src/benchmark/run_benchmarks.sh

FRAMES=100
OUTDIR="src/benchmark/results/$(date +%Y%m%d_%H%M%S)"
mkdir -p "$OUTDIR"

# Build first
echo "Building benchmarks..."
cmake -B build
cmake --build build --target benchmark_c
cmake --build build --target benchmark_cpp

scenarios=(
    "m 500 500 100"
    "m 1920 1080 100"
    "m 500 500 1000"
    "m 1920 1080 1000"
    "j 1920 1080 1000"
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
python3 src/benchmark/analyze.py "$OUTDIR"
