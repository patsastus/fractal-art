#include "fractal.hpp"
#include "utils.hpp"
#include <cmath>

// ============================================================================
// Fractal base class
// ============================================================================

void Fractal::build_sawtooth_palette() {
    colors.resize(max_iters + 1);
    for (uint32_t i = 0; i < max_iters; ++i) {
        uint32_t c0 = sawtooth(i, max_iters) * 4 * 255 / max_iters;
        uint32_t c1 = (i / 2) * 255 / max_iters;
        uint32_t c2 = (i / 4) * 255 / max_iters;
        colors[i] = (c0 << 24 | c1 << 16 | c2 << 8 | 0xFF);
    }
    colors[max_iters] = 0xFFFFFFFF;  // white for "inside the set"
}

// ============================================================================
// Mandelbrot
// ============================================================================

uint32_t Mandelbrot::iterate(Complex z) const {
    if (z.abs2() > escape_radius * escape_radius)
        return 0;
    Complex t = z;
    uint32_t n = 1;
    while (n < max_iters) {
        if (t.abs2() > escape_radius * escape_radius)
            return n;
        t = t * t + z;
        ++n;
    }
    return n;
}

void Mandelbrot::init_colors() {
    escape_radius = 2.0;
    build_sawtooth_palette();
}

// ============================================================================
// Julia
// ============================================================================

uint32_t Julia::iterate(Complex z) const {
    if (z.abs2() > escape_radius * escape_radius)
        return 0;
    Complex t = z * z + Complex(c.re, -c.im);
    uint32_t n = 1;
    while (n < max_iters) {
        if (t.abs2() > escape_radius * escape_radius)
            return n;
        t = t * t + Complex(c.re, -c.im);
        ++n;
    }
    return n;
}

double Julia::calc_escape_radius(const Complex& c) {
    double temp = c.abs();
    double r = 0.5;
    while (r * r - r <= temp)
        r *= 1.1;
    return r;
}

void Julia::init_colors() {
    escape_radius = calc_escape_radius(c);
    build_sawtooth_palette();
}

// ============================================================================
// Newton
// ============================================================================

Complex Newton::comp_function(const Complex& z) {
    // z^3 - 1
    Complex z2 = z * z;
    Complex z3 = z2 * z;
    return {z3.re - 1.0, z3.im};
}

Complex Newton::comp_deriv(const Complex& z) {
    // 3z^2
    Complex z2 = z * z;
    return {z2.re * 3.0, z2.im * 3.0};
}

int Newton::check_convergence(const Complex& z) {
    constexpr double tol = 0.0001;
    constexpr double n = 0.86602540378;  // sqrt(3) / 2

    // Solution 1: (1, 0)
    if (Complex(z.re - 1.0, z.im).abs() < tol)
        return 0;
    // Solution 2: (-0.5, sqrt(3)/2)
    if (Complex(z.re + 0.5, z.im - n).abs() < tol)
        return 1;
    // Solution 3: (-0.5, -sqrt(3)/2)
    if (Complex(z.re + 0.5, z.im + n).abs() < tol)
        return 2;
    return 4;  // not converged
}

uint32_t Newton::get_closest_root(const Complex& z) {
    constexpr double n = 0.86602540378;  // sqrt(3) / 2
    double min_dist = HUGE_VAL;
    uint32_t ret = 0;

    double d0 = Complex(z.re - 1.0, z.im).abs();
    if (d0 < min_dist) { ret = 0; min_dist = d0; }

    double d1 = Complex(z.re + 0.5, z.im - n).abs();
    if (d1 < min_dist) { ret = 1; min_dist = d1; }

    double d2 = Complex(z.re + 0.5, z.im + n).abs();
    if (d2 < min_dist) return 2;

    return ret;
}

uint32_t Newton::iterate(Complex z) const {
    Complex t = z;
    for (uint32_t n = 0; n < max_iters; ++n) {
        Complex fz = comp_function(t);
        Complex fpz = comp_deriv(t);
        Complex step = fz / fpz;
        t = t - step;

        int root = check_convergence(t);
        if (root < 3)
            return n * 3 + static_cast<uint32_t>(root);
    }
    return max_iters * 3 + get_closest_root(t);
}

void Newton::init_colors() {
    uint32_t size = (max_iters + 1) * 3;
    colors.resize(size);
    
    double max_log = std::log(max_iters + 1.0);

    for (uint32_t i = 0; i < size; i += 3) {
        uint32_t iter = i / 3;
        double normalized = std::log(iter + 1.0) / max_log;
        uint32_t depth = static_cast<uint32_t>(normalized * 255);
        
        colors[i]     = ((255 - depth) << 24 | 0xFF);       // red channel
        colors[i + 1] = ((255 - depth) << 16 | 0xFF);       // green channel
        colors[i + 2] = ((255 - depth) << 8  | 0xFF);       // blue channel
    }
}
