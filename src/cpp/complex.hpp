#ifndef COMPLEX_HPP
#define COMPLEX_HPP

#include <cmath>
#include <cstdint>

struct Anchor;

struct Complex {
    double re = 0.0;
    double im = 0.0;

    constexpr Complex() = default;
    constexpr Complex(double r, double i) : re(r), im(i) {}

    Complex operator+(const Complex& o) const { return {re + o.re, im + o.im}; }
    Complex operator-(const Complex& o) const { return {re - o.re, im - o.im}; }

    Complex operator*(const Complex& o) const {
        return {re * o.re - im * o.im,
                re * o.im + im * o.re};
    }

    Complex operator/(const Complex& o) const {
        double denom = o.re * o.re + o.im * o.im;
        return {(re * o.re + im * o.im) / denom,
                (im * o.re - re * o.im) / denom};
    }

    double abs() const { return std::sqrt(re * re + im * im); }
    double abs2() const { return re * re + im * im; }

    static Complex from_pixel(int32_t x, int32_t y, const Anchor& a);
};

struct Anchor {
    Complex value;
    int32_t x = 0;
    int32_t y = 0;
    double  px_step = 0.0;
};

#endif // COMPLEX_HPP
