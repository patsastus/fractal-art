#pragma once
#include <stdint.h>
#include "fractal.hpp"

// Device representation of a Complex number with basic operators
struct DeviceComplex {
    double re;
    double im;

    __device__ DeviceComplex operator*(const DeviceComplex& other) const {
        return {
            re * other.re - im * other.im,
            re * other.im + im * other.re
        };
    }

    __device__ DeviceComplex operator+(const DeviceComplex& other) const {
        return { re + other.re, im + other.im };
    }

    __device__ DeviceComplex operator-(const DeviceComplex& other) const {
        return { re - other.re, im - other.im };
    }
    
    __device__ DeviceComplex operator/(const DeviceComplex& other) const {
        double d = other.re * other.re + other.im * other.im;
        return {
            (re * other.re + im * other.im) / d,
            (im * other.re - re * other.im) / d
        };
    }

    __device__ double abs2() const {
        return re * re + im * im;
    }
};

// Launch a Mandelbrot kernel
void launch_mandelbrot_kernel(
    uint32_t* d_pixels,
    const uint32_t* d_colors,
    uint32_t width,
    uint32_t height,
    double anchor_re,
    double anchor_im,
    double px_step,
    uint32_t max_iters,
    double escape_radius
);

// Launch a Julia kernel
void launch_julia_kernel(
    uint32_t* d_pixels,
    const uint32_t* d_colors,
    uint32_t width,
    uint32_t height,
    double anchor_re,
    double anchor_im,
    double px_step,
    uint32_t max_iters,
    double escape_radius,
    double c_re,
    double c_im
);

// Launch a Newton kernel
void launch_newton_kernel(
    uint32_t* d_pixels,
    const uint32_t* d_colors,
    uint32_t width,
    uint32_t height,
    double anchor_re,
    double anchor_im,
    double px_step,
    uint32_t max_iters
);
