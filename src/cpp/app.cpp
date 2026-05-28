#include "app.hpp"
#include "utils.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <vector>
#include <string>

// ============================================================================
// Static callback trampolines
// ============================================================================

void App::on_key(mlx_key_data_t keydata, void* param) {
    static_cast<App*>(param)->handle_key(keydata);
}

void App::on_scroll(double xd, double yd, void* param) {
    (void)xd;
    auto* app = static_cast<App*>(param);
    int32_t x, y;
    mlx_get_mouse_pos(app->mlx_, &x, &y);
    app->handle_scroll(yd, x, y);
}

void App::on_loop(void* param) {
    auto* app = static_cast<App*>(param);
    app->frame_counter_++;
    if (app->frame_counter_ % 5 == 0 && app->fractal_->looping) {
        app->renderer_.cycle_colors(*app->fractal_);
        app->draw_scale();
        app->draw();
    }
}

void App::on_close(void* param) {
    auto* app = static_cast<App*>(param);
    mlx_close_window(app->mlx_);
}

// ============================================================================
// Construction / Destruction
// ============================================================================

App::App(int argc, char** argv) {
    fractal_ = parse_and_create(argc, argv);
    fractal_->init_colors();
    init_mlx();
    init_anchor();
    gpu_renderer_ = std::make_unique<GpuRenderer>(img_w_, img_h_);
}

App::~App() {
    if (mlx_) {
        mlx_terminate(mlx_);
    }
}

// ============================================================================
// Run
// ============================================================================

void App::run() {
    if (mlx_image_to_window(mlx_, img_, OFFS, OFFS) == -1)
        throw std::runtime_error("Failed to put image to window");
    if (mlx_image_to_window(mlx_, scale_, OFFS / 4, OFFS) == -1)
        throw std::runtime_error("Failed to put scale to window");

    mlx_scroll_hook(mlx_, on_scroll, this);
    mlx_key_hook(mlx_, on_key, this);
    mlx_close_hook(mlx_, on_close, this);
    mlx_loop_hook(mlx_, on_loop, this);

    draw_scale();
    draw();
    mlx_loop(mlx_);
}

// ============================================================================
// Init helpers
// ============================================================================

void App::init_mlx() {
    mlx_ = mlx_init(width_, height_, "Fractol", false);
    if (!mlx_)
        throw std::runtime_error("mlx_init failed");

    img_ = mlx_new_image(mlx_, img_w_, img_h_);
    uint32_t scale_h = img_h_;
    scale_ = mlx_new_image(mlx_, OFFS / 2, scale_h);

    const char* instructions = "View control: arrow keys, 'r' to reset. Loop colors: 'a', GPU toggle: 'g'";
    text_ = mlx_put_string(mlx_, instructions, OFFS, img_h_ + OFFS * 5 / 4);
    time_text_ = mlx_put_string(mlx_, "Mode: GPU | Time: -- ms", OFFS, OFFS / 2);

    if (!img_ || !scale_ || !text_ || !time_text_)
        throw std::runtime_error("Failed to create images");
}

void App::init_anchor() {
    uint32_t scale = RESOLUTION - 2 * OFFS;
    anchor_.x = scale / 2;
    anchor_.y = scale / 2;
    anchor_.px_step = fractal_->escape_radius * 3.0 / scale;
    anchor_.value = {0.0, 0.0};
}

std::unique_ptr<Fractal> App::parse_and_create(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
        throw std::runtime_error("Insufficient arguments");
    }

    uint32_t custom_iters = 0;
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-i" || arg == "--iters") {
            if (i + 1 < argc) {
                custom_iters = std::stoul(argv[++i]);
            } else {
                print_usage();
                throw std::runtime_error("Missing value for iterations flag");
            }
        } else {
            args.push_back(arg);
        }
    }

    if (args.empty()) {
        print_usage();
        throw std::runtime_error("Insufficient arguments");
    }

    char type = args[0][0];
    std::unique_ptr<Fractal> f;
    
    if (type == 'm' || type == 'M') {
        f = std::make_unique<Mandelbrot>();
    }
    else if (type == 'j' || type == 'J') {
        if (args.size() != 3) {
            print_usage();
            throw std::runtime_error("Julia requires exactly 2 parameters (re, im)");
        }
        auto julia = std::make_unique<Julia>();
        double re, im;
        if (!parse_double(args[1], re) || !parse_double(args[2], im)) {
            print_usage();
            throw std::runtime_error("Invalid Julia parameters");
        }
        julia->c = Complex(re, im);
        f = std::move(julia);
    }
    else if (type == 'n' || type == 'N') {
        f = std::make_unique<Newton>();
    }
    else {
        print_usage();
        throw std::runtime_error("Unknown fractal type: " + args[0]);
    }

    if (custom_iters > 0) {
        f->max_iters = custom_iters;
    }
    
    return f;
}

// ============================================================================
// Input handling
// ============================================================================

void App::handle_key(mlx_key_data_t keydata) {
    if (keydata.action != MLX_PRESS) return;

    switch (keydata.key) {
        case MLX_KEY_ESCAPE:
            mlx_close_window(mlx_);
            return;
        case MLX_KEY_A:
            fractal_->looping = !fractal_->looping;
            return;
        case MLX_KEY_G:
            use_gpu_ = !use_gpu_;
            break;
        case MLX_KEY_R:
            reset_view();
            break;
        case MLX_KEY_LEFT:  pan(Direction::Left);  break;
        case MLX_KEY_RIGHT: pan(Direction::Right); break;
        case MLX_KEY_UP:    pan(Direction::Up);    break;
        case MLX_KEY_DOWN:  pan(Direction::Down);  break;
        default: return;
    }
    draw();
}

void App::handle_scroll(double ydelta, int32_t x, int32_t y) {
    zoom(ydelta, x, y);
    draw();
}

void App::pan(Direction dir) {
    uint32_t xp = img_w_ / 2;
    uint32_t yp = img_h_ / 2;

    switch (dir) {
        case Direction::Left:  xp -= img_w_ / 10; break;
        case Direction::Right: xp += img_w_ / 10; break;
        case Direction::Up:    yp -= img_h_ / 10; break;
        case Direction::Down:  yp += img_h_ / 10; break;
    }

    Complex z = Complex::from_pixel(xp, yp, anchor_);
    anchor_.value = z;
    anchor_.x = img_w_ / 2;
    anchor_.y = img_h_ / 2;
}

void App::zoom(double delta, int32_t x, int32_t y) {
    uint32_t xp, yp;

    if (static_cast<uint32_t>(x) > OFFS && static_cast<uint32_t>(x) < OFFS + img_w_)
        xp = x - OFFS;
    else
        xp = img_w_ / 2;

    if (static_cast<uint32_t>(y) > OFFS && static_cast<uint32_t>(y) < OFFS + img_h_)
        yp = y - OFFS;
    else
        yp = img_h_ / 2;

    if (xp != static_cast<uint32_t>(anchor_.x) ||
        yp != static_cast<uint32_t>(anchor_.y)) {
        Complex z = Complex::from_pixel(xp, yp, anchor_);
        anchor_.value = z;
        anchor_.x = xp;
        anchor_.y = yp;
    }

    if (delta > 0 && anchor_.px_step < (HUGE_VAL / 1000))
        anchor_.px_step *= zoom_factor_;
    if (delta < 0 && anchor_.px_step > 1e-150)
        anchor_.px_step *= (2.0 - zoom_factor_);
}

void App::reset_view() {
    init_anchor();
}

// ============================================================================
// Drawing
// ============================================================================

void App::draw() {
    auto start = std::chrono::high_resolution_clock::now();

    if (use_gpu_ && gpu_renderer_) {
        if (auto m = dynamic_cast<Mandelbrot*>(fractal_.get())) {
            gpu_renderer_->render_mandelbrot(anchor_, *m);
        } else if (auto j = dynamic_cast<Julia*>(fractal_.get())) {
            gpu_renderer_->render_julia(anchor_, *j);
        } else if (auto n = dynamic_cast<Newton*>(fractal_.get())) {
            gpu_renderer_->render_newton(anchor_, *n);
        }
        gpu_renderer_->copy_to_image(img_);
    } else {
        renderer_.draw(img_, *fractal_, anchor_, img_w_, img_h_);
    }

    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;

    if (time_text_) {
        mlx_delete_image(mlx_, time_text_);
    }
    char buf[128];
    snprintf(buf, sizeof(buf), "Mode: %s | Time: %.2f ms", use_gpu_ ? "GPU" : "CPU", ms);
    time_text_ = mlx_put_string(mlx_, buf, OFFS, OFFS / 2);
}

void App::draw_scale() {
    renderer_.draw_scale(scale_, *fractal_);
}
