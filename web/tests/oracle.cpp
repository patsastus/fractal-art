#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <cstdint>

struct ComplexF32 {
    float re, im;
    ComplexF32(float r, float i) : re(r), im(i) {}
    ComplexF32 operator+(const ComplexF32& o) const { return {re + o.re, im + o.im}; }
    ComplexF32 operator-(const ComplexF32& o) const { return {re - o.re, im - o.im}; }
    ComplexF32 operator*(const ComplexF32& o) const { return {re * o.re - im * o.im, re * o.im + im * o.re}; }
    ComplexF32 operator/(const ComplexF32& o) const {
        float denom = o.re * o.re + o.im * o.im;
        return {(re * o.re + im * o.im) / denom, (im * o.re - re * o.im) / denom};
    }
    float abs2() const { return re * re + im * im; }
    float abs() const { return std::sqrt(abs2()); }
};

uint32_t mandelbrot(float c_re, float c_im, uint32_t max_iters) {
    ComplexF32 z(c_re, c_im);
    if (z.abs2() > 4.0f) return 0;
    ComplexF32 t = z;
    uint32_t n = 1;
    while (n < max_iters) {
        if (t.abs2() > 4.0f) return n;
        t = t * t + z;
        n++;
    }
    return n;
}

uint32_t julia(float z_re, float z_im, float c_re, float c_im, float radius, uint32_t max_iters) {
    ComplexF32 z(z_re, z_im);
    ComplexF32 c(c_re, -c_im); // NOTE: fractal.cpp does c(c.re, -c.im) for Julia
    if (z.abs2() > radius * radius) return 0;
    ComplexF32 t = z * z + c;
    uint32_t n = 1;
    while (n < max_iters) {
        if (t.abs2() > radius * radius) return n;
        t = t * t + c;
        n++;
    }
    return n;
}

int newton_check(const ComplexF32& z) {
    float tol = 0.0001f;
    float n = 0.86602540378f;
    if (ComplexF32(z.re - 1.0f, z.im).abs() < tol) return 0;
    if (ComplexF32(z.re + 0.5f, z.im - n).abs() < tol) return 1;
    if (ComplexF32(z.re + 0.5f, z.im + n).abs() < tol) return 2;
    return 4;
}

uint32_t newton_get_closest(const ComplexF32& z) {
    float n = 0.86602540378f;
    float min_dist = 1e10f;
    uint32_t ret = 0;
    float d0 = ComplexF32(z.re - 1.0f, z.im).abs();
    if (d0 < min_dist) { ret = 0; min_dist = d0; }
    float d1 = ComplexF32(z.re + 0.5f, z.im - n).abs();
    if (d1 < min_dist) { ret = 1; min_dist = d1; }
    float d2 = ComplexF32(z.re + 0.5f, z.im + n).abs();
    if (d2 < min_dist) return 2;
    return ret;
}

std::pair<uint32_t, uint32_t> newton(float z_re, float z_im, uint32_t max_iters) {
    ComplexF32 t(z_re, z_im);
    for (uint32_t n = 0; n < max_iters; ++n) {
        ComplexF32 t2 = t * t;
        ComplexF32 t3 = t2 * t;
        ComplexF32 fz(t3.re - 1.0f, t3.im);
        ComplexF32 fpz(t2.re * 3.0f, t2.im * 3.0f);
        ComplexF32 step = fz / fpz;
        t = t - step;
        int check = newton_check(t);
        if (check != 4) {
            return {n, check};
        }
    }
    return {max_iters, newton_get_closest(t)};
}

int main() {
    std::ofstream out("web/tests/oracle.json");
    out << "{\n";

    uint32_t width = 16, height = 16;
    float px_step = 0.05f;
    float anchor_re = -0.4f;
    float anchor_im = 0.4f;
    uint32_t max_iters = 100;

    auto get_coords = [&](uint32_t x, uint32_t y) {
        float re = anchor_re + (static_cast<float>(x) - width / 2.0f) * px_step;
        float im = anchor_im - (static_cast<float>(y) - height / 2.0f) * px_step;
        return std::make_pair(re, im);
    };

    // Mandelbrot
    out << "  \"mandelbrot\": [\n    ";
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            auto [re, im] = get_coords(x, y);
            out << mandelbrot(re, im, max_iters) << (y == height - 1 && x == width - 1 ? "" : ", ");
        }
    }
    out << "\n  ],\n";

    // Julia
    float j_re = -0.4f, j_im = 0.6f;
    float j_radius = 2.0f; // Simplified
    out << "  \"julia\": [\n    ";
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            auto [re, im] = get_coords(x, y);
            out << julia(re, im, j_re, j_im, j_radius, max_iters) << (y == height - 1 && x == width - 1 ? "" : ", ");
        }
    }
    out << "\n  ],\n";

    // Newton
    out << "  \"newton\": [\n    ";
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            auto [re, im] = get_coords(x, y);
            auto [iter, root] = newton(re, im, max_iters);
            out << "[" << iter << "," << root << "]" << (y == height - 1 && x == width - 1 ? "" : ", ");
        }
    }
    out << "\n  ]\n";
    out << "}\n";
    out.close();

    std::cout << "Oracle generated successfully.\n";
    return 0;
}
