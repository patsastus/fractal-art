# Plan 1: Refactor Bonus Implementation to C++

## Overview

Port the `bonus/` fractal renderer from C (with 42-school conventions) to modern C++ (C++20),
replacing manual memory management, raw pointers, and procedural patterns with idiomatic
C++ constructs while preserving the exact same fractal math and visual output.

---

## Current State

| File | Responsibility |
|---|---|
| `main_bonus.c` | Entry point, MLX lifecycle |
| `complex_bonus.c` | Complex number arithmetic (add, mult, div, abs) |
| `mandelbrot_bonus.c` | Mandelbrot iterator + init |
| `julia_bonus.c` | Julia iterator + init + escape radius |
| `newton_bonus.c` | Newton's method iterator + solution checking |
| `newton_utils_bonus.c` | f(z) = z³−1 and f'(z) = 3z², Newton color palette |
| `init_bonus.c` | MLX window/image init, anchor, param parsing, color palette |
| `hooks_bonus.c` | Keyboard (pan, reset, toggle animation) and scroll (zoom) handlers |
| `visuals_bonus.c` | `draw_fractal` pixel loop, color-scale bar, color cycling, text overlay |
| `utils_bonus.c` | `string_to_double`, input validation, `write_instructions`, `sawtooth` |
| `fractol_bonus.h` | Structs (`t_complex`, `t_anchor`, `t_fractal`, `t_data`), prototypes, constants |

**Key observations:**
- ~900 lines of C total
- Manual `malloc`/`free` for the color palette
- Raw function pointer `uint32_t (*iterator)(t_complex*, t_fractal*)` for polymorphism
- Single-threaded pixel-by-pixel rendering (the hot loop in `draw_fractal`)
- Depends on MLX42 (OpenGL-based, C library) for windowing and pixel buffer

---

## Proposed C++ Architecture

### 1. Complex Number Type

**Replace `t_complex` + free functions with a `Complex` struct using operator overloads.**

```cpp
// complex.hpp
struct Complex {
    double re = 0.0;
    double im = 0.0;

    Complex operator+(const Complex& o) const;
    Complex operator-(const Complex& o) const;
    Complex operator*(const Complex& o) const;
    Complex operator/(const Complex& o) const;
    double  abs() const;

    static Complex from_pixel(int32_t x, int32_t y, const Anchor& a);
};
```

> **Design note:** We use a custom struct rather than `std::complex<double>` so we
> retain full control over the layout (important for future GPU work in Plan 2) and
> keep the code self-contained. The API is nearly identical either way.

---

### 2. Fractal Hierarchy (replaces function pointer)

**Use an abstract base class for shared state and interface, with `final` on each
subclass's `iterate()` to enable devirtualization and inlining in the hot loop.**

```
Fractal (abstract)
├── Mandelbrot   (iterate is final)
├── Julia         (iterate is final)
└── Newton        (iterate is final)
```

```cpp
// fractal.hpp
class Fractal {
public:
    virtual ~Fractal() = default;
    virtual uint32_t iterate(Complex z) const = 0;
    virtual void     init_colors() = 0;

    // shared state
    double                   escape_radius;
    uint32_t                 max_iters;
    std::vector<uint32_t>    colors;

protected:
    // the sawtooth palette builder, usable by Mandelbrot & Julia
    void build_sawtooth_palette();
};

// Each subclass marks iterate() as `final` so the compiler can
// devirtualize and inline it when called through a concrete type.
class Mandelbrot : public Fractal {
public:
    uint32_t iterate(Complex z) const final;
    void     init_colors() final;
};

class Julia : public Fractal {
public:
    uint32_t iterate(Complex z) const final;
    void     init_colors() final;
    Complex  c;  // Julia parameter
};

class Newton : public Fractal {
public:
    uint32_t iterate(Complex z) const final;
    void     init_colors() final;
};
```

**Benefits:**
- No manual `malloc`/`free` — `std::vector<uint32_t>` owns the palette
- Adding a new fractal = adding a new subclass (open/closed principle)
- `final` enables the compiler to inline `iterate()` when the concrete type is
  known (see Renderer section below for how this is exploited)

---

### 3. Renderer (replaces `draw_fractal` + `visuals_bonus.c`)

**Uses template dispatch to ensure the per-pixel `iterate()` call is fully
inlined and optimizable, while keeping a clean polymorphic public API.**

```cpp
// renderer.hpp
class Renderer {
public:
    // Public API — accepts the base class, dispatches once per frame
    void draw(mlx_image_t* img, const Fractal& fractal, const Anchor& anchor);
    void draw_scale(mlx_image_t* scale_img, const Fractal& fractal);
    void cycle_colors(Fractal& fractal);

private:
    // Hot loop — templated on concrete fractal type.
    // The compiler sees the exact type, inlines iterate(), and can
    // unroll/vectorize the inner iteration loop with SIMD.
    template<typename F>
    void draw_impl(mlx_image_t* img, const F& fractal, const Anchor& anchor);
};
```

```cpp
// renderer.cpp
void Renderer::draw(mlx_image_t* img, const Fractal& fractal, const Anchor& anchor) {
    // One-time type dispatch per frame (not per pixel).
    // dynamic_cast cost is amortized over millions of pixels.
    if (auto* m = dynamic_cast<const Mandelbrot*>(&fractal))
        draw_impl(img, *m, anchor);
    else if (auto* j = dynamic_cast<const Julia*>(&fractal))
        draw_impl(img, *j, anchor);
    else if (auto* n = dynamic_cast<const Newton*>(&fractal))
        draw_impl(img, *n, anchor);
}

template<typename F>
void Renderer::draw_impl(mlx_image_t* img, const F& fractal, const Anchor& anchor) {
    // Compiler knows F is e.g. Mandelbrot, so iterate() is inlined.
    // The inner while-loop (complex_mult + abs check × max_iters)
    // becomes visible to the optimizer for unrolling and SIMD.
    for (int32_t x = 0; x < width; ++x) {
        Complex z = Complex::from_pixel(x, 0, anchor);
        for (int32_t y = 0; y < height; ++y) {
            uint32_t i = fractal.iterate(z);  // fully inlined
            mlx_put_pixel(img, x, y, fractal.colors[i]);
            z.im -= anchor.px_step;
        }
        z.re += anchor.px_step;
    }
}
```

> **Why this is faster than the original C code too:** The original C version
> used a function pointer (`data->iterator`), which is equally opaque to the
> compiler. The template approach gives us inlining that neither the C version
> nor a naive virtual call would provide.

---

### 4. Application (replaces `t_data` + `main` + `hooks`)

```cpp
// app.hpp
class App {
public:
    App(int argc, char** argv);
    ~App();
    void run();

private:
    // MLX handles
    mlx_t*       mlx_ = nullptr;
    mlx_image_t* img_ = nullptr;
    mlx_image_t* scale_ = nullptr;
    mlx_image_t* text_ = nullptr;

    // Domain objects
    std::unique_ptr<Fractal> fractal_;
    Anchor    anchor_;
    Renderer  renderer_;
    ViewState view_;  // zoom, width, height, looping flag

    // Callbacks (static trampolines → member functions)
    static void on_key(mlx_key_data_t keydata, void* param);
    static void on_scroll(double xd, double yd, void* param);
    static void on_loop(void* param);
    static void on_close(void* param);

    void handle_key(mlx_key_data_t keydata);
    void handle_scroll(double yd, int32_t x, int32_t y);
    void pan(Direction dir);
    void zoom(double delta, int32_t x, int32_t y);
    void reset_view();

    // Init helpers
    std::unique_ptr<Fractal> parse_and_create(int argc, char** argv);
};
```

> **MLX42 interop:** MLX42 is a C library. The static callback trampolines
> cast the `void*` param back to `App*` and forward to the member function.
> This is the standard pattern for wrapping C callbacks in C++.

---

### 5. Input Parsing (replaces `utils_bonus.c`)

- `string_to_double` → use `std::stod` with proper exception handling
- `check_input_arg` → regex or simple validation, throw on bad input
- `write_instructions` → `std::cerr <<` with string literals
- `ft_strlen` → removed entirely (use `std::string`)
- `sawtooth` → free function in a `math_utils.hpp` (still needed for palette)

---

## File Layout

```
src/
├── main.cpp            # Entry point, creates App and calls run()
├── complex.hpp / .cpp  # Complex struct + operators
├── fractal.hpp / .cpp  # Fractal base + Mandelbrot/Julia/Newton
├── renderer.hpp / .cpp # draw_fractal, draw_scale, cycle_colors
├── app.hpp / .cpp      # Application, hooks, init, lifecycle
└── utils.hpp / .cpp    # sawtooth, input parsing helpers
CMakeLists.txt          # Build system (replaces Makefile)
```

---

## Build System

Replace the `Makefile` with **CMake** (3.20+):

```cmake
cmake_minimum_required(VERSION 3.20)
project(fractol CXX)
set(CMAKE_CXX_STANDARD 20)

# MLX42 as a subdirectory (already cloned)
add_subdirectory(MLX42)

add_executable(fractol
    src/main.cpp
    src/complex.cpp
    src/fractal.cpp
    src/renderer.cpp
    src/app.cpp
    src/utils.cpp
)

target_link_libraries(fractol PRIVATE mlx42 glfw m dl z pthread)
target_include_directories(fractol PRIVATE src/ MLX42/include/MLX42)
target_compile_options(fractol PRIVATE -O3 -ffast-math)
```

---

## Migration Checklist

- [ ] Create `CMakeLists.txt`
- [ ] Port `t_complex` → `Complex` with operator overloads and tests
- [ ] Port `Fractal` base class with `Mandelbrot`, `Julia`, `Newton` subclasses
- [ ] Port `Renderer` (draw loop, scale, color cycling)
- [ ] Port `App` (MLX init, hooks, lifecycle)
- [ ] Port utilities (`sawtooth`, input parsing with `std::stod`)
- [ ] Verify visual output matches the original C version pixel-for-pixel
- [ ] Remove `bonus/` directory (or archive it)

---

## Risks and Open Questions

| Topic | Notes |
|---|---|
| **MLX42 C++ compatibility** | MLX42 is C. It compiles fine as `extern "C"` and the headers already use `#ifdef __cplusplus`. No changes needed. |
| **Performance at scale** | At higher resolutions (1080p+) and iteration counts (1000+), the per-pixel call to `iterate()` runs millions of times per frame, with billions of inner-loop iterations total. A virtual call would block inlining and SIMD vectorization. The template-dispatch pattern in `Renderer::draw_impl` eliminates this: the `dynamic_cast` dispatch happens once per frame, and the hot loop sees the concrete type, allowing full inlining and optimization. This should be **faster than the original C code**, which also used an opaque function pointer. |
| **42 Norm compliance** | This refactor intentionally **abandons** the 42 Norm (25-line functions, no `for`, no `//` comments, etc.). The C version in `bonus/` remains as-is for submission purposes. |
| **`std::complex` vs custom** | We use a custom struct to keep layout control for GPU porting (Plan 2). If GPU is dropped, switching to `std::complex<double>` is trivial. |
