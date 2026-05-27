#include "complex.hpp"
#include "fractal.hpp"
#include <chrono>
#include <iostream>
#include <vector>
#include <memory>
#include <string>

static unsigned long hash_buffer(const std::vector<uint32_t>& buffer) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < buffer.size(); i++) {
        hash = ((hash << 5) + hash) + buffer[i];
    }
    return hash;
}

template<typename F>
void bench_draw(std::vector<uint32_t>& buffer, uint32_t width, uint32_t height,
                const F& fractal, const Anchor& anchor) {
    Complex origin = Complex::from_pixel(0, 0, anchor);
    Complex col_start = origin;
    for (uint32_t x = 0; x < width; ++x) {
        Complex z = col_start;
        for (uint32_t y = 0; y < height; ++y) {
            uint32_t i = fractal.iterate(z);
            buffer[y * width + x] = fractal.colors[i];
            z.im -= anchor.px_step;
        }
        col_start.re += anchor.px_step;
    }
}

void run_benchmark(Fractal& fractal, uint32_t width, uint32_t height,
                   int n_frames, const Anchor& anchor) {
    std::vector<uint32_t> buffer(width * height);

    // Warmup
    for (int i = 0; i < 5; i++) {
        if (auto* m = dynamic_cast<Mandelbrot*>(&fractal))
            bench_draw(buffer, width, height, *m, anchor);
        else if (auto* j = dynamic_cast<Julia*>(&fractal))
            bench_draw(buffer, width, height, *j, anchor);
        else if (auto* n = dynamic_cast<Newton*>(&fractal))
            bench_draw(buffer, width, height, *n, anchor);
    }

    unsigned long checksum = hash_buffer(buffer);
    std::cout << "checksum," << checksum << "\n";
    std::cout << "frame,time_ms\n";

    for (int i = 0; i < n_frames; i++) {
        auto t0 = std::chrono::high_resolution_clock::now();

        if (auto* m = dynamic_cast<Mandelbrot*>(&fractal))
            bench_draw(buffer, width, height, *m, anchor);
        else if (auto* j = dynamic_cast<Julia*>(&fractal))
            bench_draw(buffer, width, height, *j, anchor);
        else if (auto* n = dynamic_cast<Newton*>(&fractal))
            bench_draw(buffer, width, height, *n, anchor);

        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        std::cout << i << "," << ms << "\n";
    }
}

int main(int argc, char** argv) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <fractal: m|j|n> <width> <height> <max_iters> [n_frames]\n";
        return 1;
    }

    char type = argv[1][0];
    uint32_t width = std::stoul(argv[2]);
    uint32_t height = std::stoul(argv[3]);
    uint32_t max_iters = std::stoul(argv[4]);
    int n_frames = argc > 5 ? std::stoi(argv[5]) : 100;

    std::unique_ptr<Fractal> fractal;
    if (type == 'm') {
        fractal = std::make_unique<Mandelbrot>();
    } else if (type == 'j') {
        auto j = std::make_unique<Julia>();
        j->c = Complex(-0.4, 0.6);
        fractal = std::move(j);
    } else if (type == 'n') {
        fractal = std::make_unique<Newton>();
    } else {
        std::cerr << "Invalid fractal type\n";
        return 1;
    }

    fractal->max_iters = max_iters;
    fractal->init_colors();

    Anchor anchor;
    uint32_t scale = 700 - 2 * 100;
    if (width > height) scale = height; else scale = width;
    
    anchor.x = width / 2;
    anchor.y = height / 2;
    anchor.px_step = fractal->escape_radius * 3.0 / scale;
    anchor.value = {0.0, 0.0};

    run_benchmark(*fractal, width, height, n_frames, anchor);

    return 0;
}
