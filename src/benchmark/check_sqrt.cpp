#include "complex.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>

int main() {
    Complex z(-1.295999999999998, 0.05999999999999747);
    
    // C++
    Complex t = z;
    uint32_t cpp_n = 1;
    std::cout << std::setprecision(18);
    
    while (cpp_n < 1000) {
        if (t.abs2() > 4.0) break;
        t = t * t + z;
        ++cpp_n;
    }
    
    // C
    t = z;
    uint32_t c_n = 1;
    while (c_n < 1000) {
        if (std::sqrt(t.re * t.re + t.im * t.im) > 2.0) break;
        t = t * t + z;
        ++c_n;
    }
    
    std::cout << "CPP Iter: " << cpp_n << "\n";
    std::cout << "C Iter:   " << c_n << "\n";
    
    // Detailed dump of C
    t = z;
    for(int i=1; i<998; ++i) {
        double mag2 = t.re * t.re + t.im * t.im;
        double mag = std::sqrt(mag2);
        if (i > 994) {
            std::cout << "Iter " << i << "\n";
            std::cout << "  mag2: " << mag2 << " (>4.0? " << (mag2 > 4.0) << ")\n";
            std::cout << "  mag:  " << mag << " (>2.0? " << (mag > 2.0) << ")\n";
        }
        t = t * t + z;
    }
    return 0;
}
