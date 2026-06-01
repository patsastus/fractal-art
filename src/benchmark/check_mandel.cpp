#include "complex.hpp"
#include "fractal.hpp"
#include <iostream>
#include <iomanip>

extern "C" {
#include <fractol.h>
}

int main() {
    Complex z(-1.295999999999998, 0.05999999999999747);
    
    // C++
    Mandelbrot cpp_m;
    cpp_m.max_iters = 1000;
    cpp_m.init_colors();
    
    Complex cpp_t = z;
    uint32_t cpp_n = 1;
    
    // C
    t_fractal f;
    f.max_iters = 1000;
    f.r = 2.0;
    
    t_complex c_z;
    c_z.re = z.re;
    c_z.im = z.im;
    
    t_complex c_t;
    set_complex(&c_t, c_z.re, c_z.im);
    uint32_t c_n = 1;

    std::cout << std::setprecision(18);

    while (cpp_n < 1000 && c_n < 1000) {
        if (cpp_t.abs2() > 4.0) break;
        if (complex_abs(&c_t) > f.r) break;
        
        cpp_t = cpp_t * cpp_t + z;
        complex_mult(&c_t, &c_t, &c_t);
        set_complex(&c_t, c_t.re + c_z.re, c_t.im + c_z.im);

        if (std::abs(cpp_t.re - c_t.re) > 1e-10 || std::abs(cpp_t.im - c_t.im) > 1e-10) {
            std::cout << "Diverged at Iteration " << cpp_n << "!\n";
            std::cout << "CPP: " << cpp_t.re << " + " << cpp_t.im << "i\n";
            std::cout << "C  : " << c_t.re << " + " << c_t.im << "i\n";
            break;
        }

        ++cpp_n;
        ++c_n;
    }
    
    return 0;
}
