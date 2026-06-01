#ifndef FRACTAL_HPP
#define FRACTAL_HPP

#include "complex.hpp"
#include <cstdint>
#include <vector>

class Fractal {
public:
    virtual ~Fractal() = default;
    virtual uint32_t iterate(Complex z) const = 0;
    virtual void     init_colors() = 0;

    double                escape_radius = 2.0;
    uint32_t              max_iters = 100;
    std::vector<uint32_t> colors;
    bool                  looping = false;

protected:
    void build_sawtooth_palette();
};

class Mandelbrot : public Fractal {
public:
    uint32_t iterate(Complex z) const final;
    void     init_colors() final;
};

class Julia : public Fractal {
public:
    uint32_t iterate(Complex z) const final;
    void     init_colors() final;

    Complex c;  // Julia parameter

    // Calculate escape radius for the given c
    static double calc_escape_radius(const Complex& c);
};

class Newton : public Fractal {
public:
    uint32_t iterate(Complex z) const final;
    void     init_colors() final;

    Newton() { max_iters = 30; }  // Newton needs fewer iterations

private:
    // f(z) = z^3 - 1
    static Complex comp_function(const Complex& z);
    // f'(z) = 3z^2
    static Complex comp_deriv(const Complex& z);
    // Check if z is close to one of the 3 known roots, returns 0/1/2 or 4 if not
    static int check_convergence(const Complex& z);
    // Returns the index of the closest root
    static uint32_t get_closest_root(const Complex& z);
};

#endif // FRACTAL_HPP
