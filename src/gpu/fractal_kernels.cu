#include "fractal_kernels.cuh"
#include <cuda_runtime.h>
#include <math.h>

__device__ uint32_t get_rgba(uint32_t color) {
    return ((color & 0xFF) << 24) |
           ((color & 0xFF00) << 8) |
           ((color & 0xFF0000) >> 8) |
           ((color >> 24) & 0xFF);
}

__global__ void mandelbrot_kernel(
    uint32_t* pixels, const uint32_t* colors,
    uint32_t width, uint32_t height,
    double anchor_re, double anchor_im, double px_step,
    uint32_t max_iters, double er2
) {
    uint32_t x = blockIdx.x * blockDim.x + threadIdx.x;
    uint32_t y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= width || y >= height) return;

    DeviceComplex z = { anchor_re + x * px_step, anchor_im - y * px_step };

    if (z.abs2() > er2) {
        pixels[y * width + x] = get_rgba(colors[0]);
        return;
    }

    DeviceComplex t = z;
    uint32_t iter = 1;
    while (iter < max_iters) {
        if (t.abs2() > er2)
            break;
        t = t * t + z;
        iter++;
    }

    pixels[y * width + x] = get_rgba(colors[iter]);
}

void launch_mandelbrot_kernel(
    uint32_t* d_pixels, const uint32_t* d_colors,
    uint32_t width, uint32_t height,
    double anchor_re, double anchor_im, double px_step,
    uint32_t max_iters, double escape_radius
) {
    dim3 threads(16, 16);
    dim3 blocks((width + 15) / 16, (height + 15) / 16);
    mandelbrot_kernel<<<blocks, threads>>>(
        d_pixels, d_colors, width, height,
        anchor_re, anchor_im, px_step, max_iters, escape_radius * escape_radius
    );
}

__global__ void julia_kernel(
    uint32_t* pixels, const uint32_t* colors,
    uint32_t width, uint32_t height,
    double anchor_re, double anchor_im, double px_step,
    uint32_t max_iters, double er2, double c_re, double c_im
) {
    uint32_t x = blockIdx.x * blockDim.x + threadIdx.x;
    uint32_t y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= width || y >= height) return;

    DeviceComplex z = { anchor_re + x * px_step, anchor_im - y * px_step };
    DeviceComplex c_val = { c_re, -c_im };

    if (z.abs2() > er2) {
        pixels[y * width + x] = get_rgba(colors[0]);
        return;
    }

    DeviceComplex t = z * z + c_val;
    uint32_t iter = 1;
    while (iter < max_iters) {
        if (t.abs2() > er2)
            break;
        t = t * t + c_val;
        iter++;
    }

    pixels[y * width + x] = get_rgba(colors[iter]);
}

void launch_julia_kernel(
    uint32_t* d_pixels, const uint32_t* d_colors,
    uint32_t width, uint32_t height,
    double anchor_re, double anchor_im, double px_step,
    uint32_t max_iters, double escape_radius,
    double c_re, double c_im
) {
    dim3 threads(16, 16);
    dim3 blocks((width + 15) / 16, (height + 15) / 16);
    julia_kernel<<<blocks, threads>>>(
        d_pixels, d_colors, width, height,
        anchor_re, anchor_im, px_step, max_iters, escape_radius * escape_radius,
        c_re, c_im
    );
}

// Newton math helpers
__device__ DeviceComplex comp_function(const DeviceComplex& z) {
    DeviceComplex z2 = z * z;
    DeviceComplex z3 = z2 * z;
    return {z3.re - 1.0, z3.im};
}

__device__ DeviceComplex comp_deriv(const DeviceComplex& z) {
    DeviceComplex z2 = z * z;
    return {z2.re * 3.0, z2.im * 3.0};
}

__device__ int check_convergence(const DeviceComplex& z) {
    const double tol = 0.0001;
    const double n = 0.86602540378; // sqrt(3) / 2

    DeviceComplex d1 = {z.re - 1.0, z.im};
    if (d1.abs2() < tol * tol) return 0;

    DeviceComplex d2 = {z.re + 0.5, z.im - n};
    if (d2.abs2() < tol * tol) return 1;

    DeviceComplex d3 = {z.re + 0.5, z.im + n};
    if (d3.abs2() < tol * tol) return 2;

    return 4; // not converged
}

__device__ uint32_t get_closest_root(const DeviceComplex& z) {
    const double n = 0.86602540378;
    double min_dist = 1e30;
    uint32_t ret = 0;

    double dist1 = DeviceComplex{z.re - 1.0, z.im}.abs2();
    if (dist1 < min_dist) { ret = 0; min_dist = dist1; }

    double dist2 = DeviceComplex{z.re + 0.5, z.im - n}.abs2();
    if (dist2 < min_dist) { ret = 1; min_dist = dist2; }

    double dist3 = DeviceComplex{z.re + 0.5, z.im + n}.abs2();
    if (dist3 < min_dist) { return 2; }

    return ret;
}

__global__ void newton_kernel(
    uint32_t* pixels, const uint32_t* colors,
    uint32_t width, uint32_t height,
    double anchor_re, double anchor_im, double px_step,
    uint32_t max_iters
) {
    uint32_t x = blockIdx.x * blockDim.x + threadIdx.x;
    uint32_t y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= width || y >= height) return;

    DeviceComplex z = { anchor_re + x * px_step, anchor_im - y * px_step };
    DeviceComplex t = z;

    for (uint32_t iter = 0; iter < max_iters; ++iter) {
        DeviceComplex fz = comp_function(t);
        DeviceComplex fpz = comp_deriv(t);
        DeviceComplex step = fz / fpz;
        t = t - step;

        int root = check_convergence(t);
        if (root < 3) {
            pixels[y * width + x] = get_rgba(colors[iter * 3 + root]);
            return;
        }
    }

    pixels[y * width + x] = get_rgba(colors[max_iters * 3 + get_closest_root(t)]);
}

void launch_newton_kernel(
    uint32_t* d_pixels, const uint32_t* d_colors,
    uint32_t width, uint32_t height,
    double anchor_re, double anchor_im, double px_step,
    uint32_t max_iters
) {
    dim3 threads(16, 16);
    dim3 blocks((width + 15) / 16, (height + 15) / 16);
    newton_kernel<<<blocks, threads>>>(
        d_pixels, d_colors, width, height,
        anchor_re, anchor_im, px_step, max_iters
    );
}
