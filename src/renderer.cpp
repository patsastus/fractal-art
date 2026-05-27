#include "renderer.hpp"

void Renderer::draw(mlx_image_t* img, const Fractal& fractal,
                    const Anchor& anchor, uint32_t width, uint32_t height) {
    // One-time type dispatch per frame (not per pixel).
    if (auto* m = dynamic_cast<const Mandelbrot*>(&fractal))
        draw_impl(img, *m, anchor, width, height);
    else if (auto* j = dynamic_cast<const Julia*>(&fractal))
        draw_impl(img, *j, anchor, width, height);
    else if (auto* n = dynamic_cast<const Newton*>(&fractal))
        draw_impl(img, *n, anchor, width, height);
}

void Renderer::draw_scale(mlx_image_t* scale_img, const Fractal& fractal,
                           uint32_t img_h) {
    size_t palette_size = fractal.colors.size();
    if (palette_size == 0) return;

    uint32_t scale_w = scale_img->width;
    uint32_t height = (img_h / palette_size) * palette_size;

    for (uint32_t x = 0; x < scale_w; ++x) {
        for (uint32_t y = 0; y < height; ++y) {
            size_t i = y * palette_size / height;
            mlx_put_pixel(scale_img, x, y, fractal.colors[i]);
        }
    }
}

void Renderer::cycle_colors(Fractal& fractal) {
    if (fractal.colors.size() < 2) return;

    uint32_t temp = fractal.colors[0];
    size_t last = fractal.colors.size() - 2;  // preserve the last color
    for (size_t i = 0; i < last; ++i)
        fractal.colors[i] = fractal.colors[i + 1];
    fractal.colors[last] = temp;
}
