#include <fractol.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long hash_buffer(uint32_t *buffer, size_t count) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < count; i++) {
        hash = ((hash << 5) + hash) + buffer[i];
    }
    return hash;
}

static void bench_draw(uint32_t* buffer, uint32_t width, uint32_t height,
                        t_fractal* f, t_anchor* a,
                        uint32_t (*iterator)(t_complex*, t_fractal*)) {
    int32_t x = 0;
    t_complex temp[2];

    pixel_to_complex(&temp[0], 0, 0, a);
    set_complex(&temp[1], temp[0].re, temp[0].im);
    while ((uint32_t)x < width) {
        int32_t y = 0;
        while ((uint32_t)y < height) {
            uint32_t i = iterator(&temp[1], f);
            buffer[y * width + x] = f->colors[i];
            set_complex(&temp[1], temp[1].re, temp[1].im - a->px_step);
            ++y;
        }
        set_complex(&temp[1], temp[1].re + a->px_step, temp[0].im);
        ++x;
    }
}

static double time_ms(struct timespec* start, struct timespec* end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 +
           (end->tv_nsec - start->tv_nsec) / 1e6;
}

// --- Stubs to satisfy linker for unused functions in init.c / visuals.c ---
mlx_t* mlx_init(int32_t w, int32_t h, const char* title, bool resize) { return NULL; }
mlx_image_t* mlx_new_image(mlx_t* mlx, uint32_t width, uint32_t height) { return NULL; }
void make_text(t_data* data, int32_t x, int32_t y) {}
void make_scale(t_data* data) {}
void ft_exit(void* param) { exit(1); }
mlx_image_t* mlx_put_string(mlx_t* mlx, const char* str, int32_t x, int32_t y) { return NULL; }
// --------------------------------------------------------------------------

int main(int argc, char** argv) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <fractal: m|j|n> <width> <height> <max_iters> [n_frames]\n", argv[0]);
        return 1;
    }

    char type = argv[1][0];
    uint32_t width = atoi(argv[2]);
    uint32_t height = atoi(argv[3]);
    uint32_t max_iters = atoi(argv[4]);
    int n_frames = argc > 5 ? atoi(argv[5]) : 100;

    t_data data;
    memset(&data, 0, sizeof(data));
    data.f.max_iters = max_iters;
    data.f.colors = NULL;
    data.f.size = 0;

    uint32_t (*iterator)(t_complex*, t_fractal*) = NULL;

    if (type == 'm') {
        data.f.r = 2.0;
        iterator = iter_mandelbrot;
        make_colors(&data.f);
    } else if (type == 'j') {
        data.f.c.re = -0.4;
        data.f.c.im = 0.6;
        data.f.r = calc_julia_radius(&data.f.c);
        iterator = iter_julia;
        make_colors(&data.f);
    } else if (type == 'n') {
        data.f.r = 2.0;
        iterator = iter_newton;
        make_colors_newton(&data.f);
    } else {
        fprintf(stderr, "Invalid fractal type\n");
        return 1;
    }

    uint32_t scale = 700 - 2 * 100; // Original RESOLUTION - 2*OFFS
    if (width > height) scale = height; else scale = width;
    init_anchor(&data.a, width / 2, height / 2, data.f.r * 3.0 / scale);

    uint32_t* buffer = malloc(sizeof(uint32_t) * width * height);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate buffer\n");
        return 1;
    }

    struct timespec t0, t1;

    // Warmup
    for (int i = 0; i < 5; i++)
        bench_draw(buffer, width, height, &data.f, &data.a, iterator);

    unsigned long checksum = hash_buffer(buffer, width * height);
    printf("checksum,%lu\n", checksum);
    printf("frame,time_ms\n");

    for (int i = 0; i < n_frames; i++) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        bench_draw(buffer, width, height, &data.f, &data.a, iterator);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("%d,%.3f\n", i, time_ms(&t0, &t1));
    }

    free(buffer);
    if (data.f.colors) free(data.f.colors);
    return 0;
}
