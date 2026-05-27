#include "complex.hpp"

Complex Complex::from_pixel(int32_t x, int32_t y, const Anchor& a) {
    return {a.value.re + (x - a.x) * a.px_step,
            a.value.im + (a.y - y) * a.px_step};
}
