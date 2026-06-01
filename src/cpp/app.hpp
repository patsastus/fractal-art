#ifndef APP_HPP
#define APP_HPP

#include "complex.hpp"
#include "fractal.hpp"
#include "renderer.hpp"
#include "../gpu/gpu_renderer.cuh"
#include <memory>
#include <cstdint>

extern "C" {
#include <MLX42.h>
}

constexpr uint32_t RESOLUTION = 700;
constexpr uint32_t OFFS = 100;

enum class Direction { Left, Right, Up, Down };

class App {
public:
    App(int argc, char** argv);
    ~App();

    void run();

private:
    // MLX handles
    mlx_t*       mlx_   = nullptr;
    mlx_image_t* img_   = nullptr;
    mlx_image_t* scale_ = nullptr;
    mlx_image_t* text_  = nullptr;
    mlx_image_t* time_text_ = nullptr;

    // Domain objects
    std::unique_ptr<Fractal> fractal_;
    Anchor    anchor_;
    Renderer  renderer_;
    std::unique_ptr<GpuRenderer> gpu_renderer_;
    bool      use_gpu_ = true;

    // View state
    double   zoom_factor_ = 1.1;
    uint32_t width_       = RESOLUTION;
    uint32_t height_      = RESOLUTION;
    uint32_t img_w_       = RESOLUTION - 2 * OFFS;
    uint32_t img_h_       = RESOLUTION - 2 * OFFS;

    // Frame counter for color cycling throttle
    uint8_t frame_counter_ = 0;

    // Callbacks (static trampolines → member functions)
    static void on_key(mlx_key_data_t keydata, void* param);
    static void on_scroll(double xd, double yd, void* param);
    static void on_resize(int32_t width, int32_t height, void* param);
    static void on_loop(void* param);
    static void on_close(void* param);

    // Input handling
    void handle_key(mlx_key_data_t keydata);
    void handle_scroll(double ydelta, int32_t x, int32_t y);
    void handle_resize(int32_t width, int32_t height);
    void pan(Direction dir);
    void zoom(double delta, int32_t x, int32_t y);
    void reset_view();

    // Drawing
    void draw();
    void draw_scale();

    // Init helpers
    void init_mlx();
    void init_anchor();
    std::unique_ptr<Fractal> parse_and_create(int argc, char** argv);
};

#endif // APP_HPP
