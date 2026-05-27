#include "../cpp/fractal.hpp"
#include "../cpp/renderer.hpp"
#include "../gpu/gpu_renderer.cuh"
#include "../cpp/complex.hpp"
#include <iostream>
#include <vector>
#include <MLX42.h>

void test_parity(const std::string& name, Fractal* f, Anchor a) {
    uint32_t w = 500;
    uint32_t h = 500;
    f->init_colors();

    // CPU buffer
    std::vector<uint8_t> cpu_pixels(w * h * 4, 0);
    mlx_image_t cpu_img = {w, h, cpu_pixels.data(), nullptr, 0, true, nullptr};
    cpu_img.pixels = cpu_pixels.data();
    Renderer cpu_r;
    cpu_r.draw(&cpu_img, *f, a, w, h);

    // GPU buffer
    std::vector<uint8_t> gpu_pixels(w * h * 4, 0);
    mlx_image_t gpu_img = {w, h, gpu_pixels.data(), nullptr, 0, true, nullptr};
    GpuRenderer gpu_r(w, h);
    
    if (auto m = dynamic_cast<Mandelbrot*>(f)) {
        gpu_r.render_mandelbrot(a, *m);
    } else if (auto j = dynamic_cast<Julia*>(f)) {
        gpu_r.render_julia(a, *j);
    } else if (auto n = dynamic_cast<Newton*>(f)) {
        gpu_r.render_newton(a, *n);
    }
    gpu_r.copy_to_image(&gpu_img);

    // Compare
    int mismatches = 0;
    for (size_t i = 0; i < cpu_pixels.size(); i += 4) {
        // Compare the 32-bit pixel value
        uint32_t p1 = *reinterpret_cast<uint32_t*>(&cpu_pixels[i]);
        uint32_t p2 = *reinterpret_cast<uint32_t*>(&gpu_pixels[i]);
        if (p1 != p2) {
            if (mismatches < 5) {
                uint32_t px = (i / 4) % w;
                uint32_t py = (i / 4) / w;
                std::cout << "  Mismatch at (" << px << ", " << py << "): CPU=" 
                          << p1 << ", GPU=" << p2 << "\n";
            }
            mismatches++;
        }
    }

    if (mismatches == 0) {
        std::cout << "[PASS] " << name << " parity verified.\n";
    } else {
        std::cout << "[FAIL] " << name << " had " << mismatches << " divergent pixels!\n";
    }
}

int main() {
    Anchor a;
    a.x = 250;
    a.y = 250;
    a.value = {0.0, 0.0};
    a.px_step = 0.005;

    Mandelbrot m;
    m.max_iters = 100;
    test_parity("Mandelbrot", &m, a);

    Julia j;
    j.max_iters = 100;
    j.c = {-0.8, 0.156};
    test_parity("Julia", &j, a);

    Newton n;
    n.max_iters = 30;
    test_parity("Newton", &n, a);

    return 0;
}
