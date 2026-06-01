#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "complex.hpp"
#include "fractal.hpp"

extern "C" {
#include <MLX42.h>
}

class Renderer {
public:
    // Public API — accepts the base class, dispatches once per frame
    void draw(mlx_image_t* img, const Fractal& fractal, const Anchor& anchor,
              uint32_t width, uint32_t height);
    void draw_scale(mlx_image_t* scale_img, const Fractal& fractal);
    void cycle_colors(Fractal& fractal);

private:
    // Hot loop — templated on concrete fractal type.
    // The compiler sees the exact type, inlines iterate(), and can
    // unroll/vectorize the inner iteration loop with SIMD.
    template<typename F>
    void draw_impl(mlx_image_t* img, const F& fractal, const Anchor& anchor,
                   uint32_t width, uint32_t height);
};

// Template definition must be visible to callers — defined here in the header.
template<typename F>
void Renderer::draw_impl(mlx_image_t* img, const F& fractal,
                          const Anchor& anchor,
                          uint32_t width, uint32_t height) {
    // Top-left pixel in the complex plane
    Complex origin = Complex::from_pixel(0, 0, anchor);
    Complex col_start = origin;

    for (uint32_t x = 0; x < width; ++x) {
        Complex z = col_start;
        for (uint32_t y = 0; y < height; ++y) {
            uint32_t i = fractal.iterate(z);  // fully inlined
            mlx_put_pixel(img, x, y, fractal.colors[i]);
            z.im -= anchor.px_step;
        }
        col_start.re += anchor.px_step;
    }
}

#endif // RENDERER_HPP
