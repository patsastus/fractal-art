cmake_minimum_required(VERSION 3.20)
set(CMAKE_CUDA_HOST_COMPILER /usr/bin/g++-12)
project(fractol CXX C CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# MLX42 as a subdirectory (already cloned or will be cloned by the old Makefile)
add_subdirectory(MLX42)

add_executable(fractol
    src/cpp/main.cpp
    src/cpp/complex.cpp
    src/cpp/fractal.cpp
    src/cpp/renderer.cpp
    src/cpp/app.cpp
    src/cpp/utils.cpp
)

add_library(fractal_cuda STATIC
    src/gpu/gpu_renderer.cu
    src/gpu/fractal_kernels.cu
)
target_include_directories(fractal_cuda PRIVATE src/cpp/ include/ MLX42/include/MLX42)
set_target_properties(fractal_cuda PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

target_link_libraries(fractol PRIVATE fractal_cuda mlx42 glfw m dl z pthread)
target_include_directories(fractol PRIVATE
    src/cpp/
    MLX42/include/MLX42
)
target_compile_options(fractol PRIVATE -O3 -ffast-math -Wall -Wextra)

# --- Benchmarks ---

# C benchmark
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
target_include_directories(benchmark_c PRIVATE include/ MLX42/include/MLX42)
target_compile_options(benchmark_c PRIVATE -O3)
target_link_libraries(benchmark_c PRIVATE m)

# C++ benchmark
add_executable(benchmark_cpp
    src/benchmark/benchmark_cpp.cpp
    src/cpp/complex.cpp
    src/cpp/fractal.cpp
    src/cpp/utils.cpp
)
target_include_directories(benchmark_cpp PRIVATE src/cpp/)
target_compile_options(benchmark_cpp PRIVATE -O3)
target_link_libraries(benchmark_cpp PRIVATE m)

# divergence finder
add_executable(divergence_finder
    src/benchmark/divergence_finder.cpp
    src/cpp/complex.cpp
    src/cpp/fractal.cpp
    src/cpp/utils.cpp
    src/c/complex.c
    src/c/mandelbrot.c
    src/c/julia.c
    src/c/newton.c
    src/c/newton_utils.c
    src/c/utils.c
    src/c/init.c
)
target_include_directories(divergence_finder PRIVATE src/cpp/ include/ MLX42/include/MLX42)
target_compile_options(divergence_finder PRIVATE -O3)
target_link_libraries(divergence_finder PRIVATE m)

# gpu parity test
add_executable(test_gpu_parity
    src/benchmark/test_gpu_parity.cpp
    src/cpp/complex.cpp
    src/cpp/fractal.cpp
    src/cpp/utils.cpp
    src/cpp/renderer.cpp
)
target_include_directories(test_gpu_parity PRIVATE src/cpp/ include/ MLX42/include/MLX42)
target_compile_options(test_gpu_parity PRIVATE -O3)
target_link_libraries(test_gpu_parity PRIVATE fractal_cuda m mlx42 glfw dl z pthread)
