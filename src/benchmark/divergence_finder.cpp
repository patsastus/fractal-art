#include "complex.hpp"
#include "fractal.hpp"
#include <iostream>
#include <iomanip>
#include <memory>

extern "C" {
#include <fractol.h>
mlx_t* mlx_init(int32_t w, int32_t h, const char* title, bool resize) { return NULL; }
mlx_image_t* mlx_new_image(mlx_t* mlx, uint32_t width, uint32_t height) { return NULL; }
void make_text(t_data* data, int32_t x, int32_t y) {}
void make_scale(t_data* data) {}
void ft_exit(void* param) { exit(1); }
mlx_image_t* mlx_put_string(mlx_t* mlx, const char* str, int32_t x, int32_t y) { return NULL; }
}

void test_divergence(char type, uint32_t width, uint32_t height, uint32_t max_iters) {
    // Setup C++
    std::unique_ptr<Fractal> cpp_fractal;
    if (type == 'm') {
        cpp_fractal = std::make_unique<Mandelbrot>();
    } else if (type == 'j') {
        auto j = std::make_unique<Julia>();
        j->c = Complex(-0.4, 0.6);
        cpp_fractal = std::move(j);
    }
    cpp_fractal->max_iters = max_iters;
    cpp_fractal->init_colors();

    Anchor cpp_anchor;
    uint32_t scale = 700 - 2 * 100;
    if (width > height) scale = height; else scale = width;
    cpp_anchor.x = width / 2;
    cpp_anchor.y = height / 2;
    cpp_anchor.px_step = cpp_fractal->escape_radius * 3.0 / scale;
    cpp_anchor.value = {0.0, 0.0};

    // Setup C
    t_data c_data;
    __builtin_memset(&c_data, 0, sizeof(c_data));
    c_data.f.max_iters = max_iters;
    
    uint32_t (*c_iterator)(t_complex*, t_fractal*) = nullptr;
    if (type == 'm') {
        c_data.f.r = 2.0;
        c_iterator = iter_mandelbrot;
    } else if (type == 'j') {
        c_data.f.c.re = -0.4;
        c_data.f.c.im = 0.6;
        c_data.f.r = calc_julia_radius(&c_data.f.c);
        c_iterator = iter_julia;
    }
    init_anchor(&c_data.a, width / 2, height / 2, c_data.f.r * 3.0 / scale);

    // Compare
    Complex origin = Complex::from_pixel(0, 0, cpp_anchor);
    Complex cpp_z = origin;
    
    t_complex c_z;
    pixel_to_complex(&c_z, 0, 0, &c_data.a);
    
    std::cout << std::setprecision(16);
    std::cout << "Origin CPP: " << origin.re << " + " << origin.im << "i\n";
    std::cout << "Origin C:   " << c_z.re << " + " << c_z.im << "i\n";

    for (uint32_t x = 0; x < width; ++x) {
        Complex z = cpp_z;
        t_complex cz = c_z;
        
        for (uint32_t y = 0; y < height; ++y) {
            uint32_t cpp_i = cpp_fractal->iterate(z);
            uint32_t c_i = c_iterator(&cz, &c_data.f);
            
            if (cpp_i != c_i) {
                std::cout << "Divergence at (" << x << ", " << y << ")!\n";
                std::cout << "Z cpp: " << z.re << " + " << z.im << "i\n";
                std::cout << "Z c:   " << cz.re << " + " << cz.im << "i\n";
                std::cout << "CPP Iter: " << cpp_i << "\n";
                std::cout << "C Iter:   " << c_i << "\n";
                return;
            }
            z.im -= cpp_anchor.px_step;
            cz.im -= c_data.a.px_step;
        }
        cpp_z.re += cpp_anchor.px_step;
        c_z.re += c_data.a.px_step;
    }
    std::cout << "No divergence found for type " << type << " at " << max_iters << " iterations.\n";
}

int main() {
    std::cout << "Testing Mandelbrot Deep (500x500x1000)...\n";
    test_divergence('m', 500, 500, 1000);
    
    std::cout << "\nTesting Julia Deep (1920x1080x1000)...\n";
    test_divergence('j', 1920, 1080, 1000);
    
    return 0;
}
