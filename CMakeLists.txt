cmake_minimum_required(VERSION 3.20)
project(fractol CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MLX42 as a subdirectory (already cloned or will be cloned by the old Makefile)
add_subdirectory(MLX42)

add_executable(fractol
    src/main.cpp
    src/complex.cpp
    src/fractal.cpp
    src/renderer.cpp
    src/app.cpp
    src/utils.cpp
)

target_link_libraries(fractol PRIVATE mlx42 glfw m dl z pthread)
target_include_directories(fractol PRIVATE
    src/
    MLX42/include/MLX42
)
target_compile_options(fractol PRIVATE -O3 -ffast-math -Wall -Wextra)
