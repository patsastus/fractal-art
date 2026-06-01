#include "gpu_renderer.cuh"
#include "fractal_kernels.cuh"
#include <cuda_runtime.h>
#include <iostream>
#include <string.h>

#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            std::cerr << "CUDA error at " << __FILE__ << ":" << __LINE__ \
                      << " code=" << err << " \"" << cudaGetErrorString(err) << "\"\n"; \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

GpuRenderer::GpuRenderer(uint32_t width, uint32_t height) 
    : width_(width), height_(height) {
    pixel_bytes_ = width_ * height_ * sizeof(uint32_t);

    CUDA_CHECK(cudaMalloc(&d_pixels_, pixel_bytes_));
    h_pixels_ = new uint32_t[width_ * height_];

    // Allocate some max size for colors
    CUDA_CHECK(cudaMalloc(&d_colors_, 4000 * sizeof(uint32_t))); 
}

GpuRenderer::~GpuRenderer() {
    cudaFree(d_pixels_);
    cudaFree(d_colors_);
    delete[] h_pixels_;
}

void GpuRenderer::ensure_colors_uploaded(const Fractal& fractal) {
    if (current_colors_ != fractal.colors) {
        current_colors_ = fractal.colors;
        CUDA_CHECK(cudaMemcpy(d_colors_, fractal.colors.data(), 
                   fractal.colors.size() * sizeof(uint32_t), cudaMemcpyHostToDevice));
    }
}

void GpuRenderer::render_mandelbrot(const Anchor& anchor, const Mandelbrot& fractal) {
    ensure_colors_uploaded(fractal);
    Complex origin = Complex::from_pixel(0, 0, anchor);
    launch_mandelbrot_kernel(
        d_pixels_, d_colors_, width_, height_,
        origin.re, origin.im, anchor.px_step,
        fractal.max_iters, fractal.escape_radius
    );
    CUDA_CHECK(cudaGetLastError());
}

void GpuRenderer::render_julia(const Anchor& anchor, const Julia& fractal) {
    ensure_colors_uploaded(fractal);
    Complex origin = Complex::from_pixel(0, 0, anchor);
    launch_julia_kernel(
        d_pixels_, d_colors_, width_, height_,
        origin.re, origin.im, anchor.px_step,
        fractal.max_iters, fractal.escape_radius,
        fractal.c.re, fractal.c.im
    );
    CUDA_CHECK(cudaGetLastError());
}

void GpuRenderer::render_newton(const Anchor& anchor, const Newton& fractal) {
    ensure_colors_uploaded(fractal);
    Complex origin = Complex::from_pixel(0, 0, anchor);
    launch_newton_kernel(
        d_pixels_, d_colors_, width_, height_,
        origin.re, origin.im, anchor.px_step,
        fractal.max_iters
    );
    CUDA_CHECK(cudaGetLastError());
}

void GpuRenderer::copy_to_image(mlx_image_t* img) {
    // Wait for kernels to finish and copy to host staging buffer
    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaMemcpy(h_pixels_, d_pixels_, pixel_bytes_, cudaMemcpyDeviceToHost));
    
    // MLX42 image buffer format is RGBA bytes. 
    // Wait, the kernel wrote uint32_t colors.
    // If the host architecture is little-endian, a uint32_t color in the array 
    // will be copied directly as exactly the same byte ordering.
    // We just memcpy the host staging buffer to the mlx image pixels.
    memcpy(img->pixels, h_pixels_, pixel_bytes_);
}
