#pragma once

#include <stdint.h>
#include <vector>
#include "fractal.hpp"
#include <MLX42.h>

class GpuRenderer {
public:
    GpuRenderer(uint32_t width, uint32_t height);
    ~GpuRenderer();

    // Disable copy/move for simplicity
    GpuRenderer(const GpuRenderer&) = delete;
    GpuRenderer& operator=(const GpuRenderer&) = delete;

    void render_mandelbrot(const Anchor& anchor, const Mandelbrot& fractal);
    void render_julia(const Anchor& anchor, const Julia& fractal);
    void render_newton(const Anchor& anchor, const Newton& fractal);

    // Copy result back to MLX image
    void copy_to_image(mlx_image_t* img);

private:
    void ensure_colors_uploaded(const Fractal& fractal);

    uint32_t width_;
    uint32_t height_;
    size_t pixel_bytes_;
    
    uint32_t* d_pixels_;
    uint32_t* d_colors_;
    uint32_t* h_pixels_; // Host staging buffer

    // Keep track of current uploaded palette to avoid redundant copies
    std::vector<uint32_t> current_colors_;
};
