# Plan 2: GPU-Accelerated Fractal Computation (CUDA / Compute Shaders)

## Overview

The current fractal renderer is **single-threaded** — it loops over every pixel
sequentially in `draw_fractal`. Fractal computation is *embarrassingly parallel*:
each pixel's iteration count is completely independent of every other pixel. This
makes it an ideal candidate for GPU acceleration.

This plan covers two approaches (which can coexist):
1. **CUDA** — for NVIDIA GPUs on the desktop
2. **OpenGL Compute Shaders** — for cross-vendor GPU compute via MLX42's existing OpenGL context

---

## Why GPU?

Current performance profile of `draw_fractal`:

| Parameter | Value |
|---|---|
| Image size | 500 × 500 = 250,000 pixels |
| Max iterations (Mandelbrot/Julia) | 100 |
| Max iterations (Newton) | 30 |
| Work per pixel | 1 `complex_abs` + 1 `complex_mult` + 1 `set_complex` per iteration |
| Total operations | ~250k × 100 = **25M** complex operations (Mandelbrot) |

On a single CPU core, this is noticeable (~50–150ms per frame depending on zoom).
On a GPU with thousands of cores, each pixel is one thread — the whole frame
computes in <1ms at this resolution, allowing for:
- Higher resolution (1080p, 4K)
- Higher max iterations (1000+) for deep zooms
- Smooth real-time zooming and panning

---

## Approach A: CUDA Kernels

### Prerequisites
- NVIDIA GPU with Compute Capability 3.5+
- CUDA Toolkit 12+
- `nvcc` compiler

### Kernel Design

```cuda
// fractal_kernel.cu

struct Complex {
    double re, im;
    __device__ Complex operator*(const Complex& o) const { /* ... */ }
    __device__ double abs2() const { return re*re + im*im; }  // avoid sqrt
};

__global__ void mandelbrot_kernel(
    uint32_t* pixel_buffer,    // output: RGBA per pixel
    const uint32_t* palette,   // color lookup table
    double anchor_re,          // top-left complex value
    double anchor_im,
    double px_step,            // complex units per pixel
    uint32_t width,
    uint32_t height,
    uint32_t max_iters
) {
    uint32_t x = blockIdx.x * blockDim.x + threadIdx.x;
    uint32_t y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= width || y >= height) return;

    Complex c = { anchor_re + x * px_step,
                  anchor_im - y * px_step };
    Complex z = {0.0, 0.0};

    uint32_t iter = 0;
    while (iter < max_iters && z.abs2() < 4.0) {
        z = z * z + c;
        iter++;
    }

    pixel_buffer[y * width + x] = palette[iter];
}
```

> **Key optimization:** Use `abs2()` (squared magnitude) instead of `abs()` to
> avoid a `sqrt` per iteration. Compare against `4.0` instead of `2.0`.
> This is a free 10–15% speedup.

### Host-Side Integration

```cpp
// gpu_renderer.hpp
class GpuRenderer {
public:
    GpuRenderer(uint32_t width, uint32_t height);
    ~GpuRenderer();

    void render_mandelbrot(const Anchor& anchor, const Fractal& fractal);
    void render_julia(const Anchor& anchor, const Julia& fractal);
    void render_newton(const Anchor& anchor, const Newton& fractal);

    // Copy result back to MLX image
    void copy_to_image(mlx_image_t* img);

private:
    uint32_t  width_, height_;
    uint32_t* d_pixels_  = nullptr;  // device pixel buffer
    uint32_t* d_palette_ = nullptr;  // device color palette
    uint32_t* h_pixels_  = nullptr;  // host staging buffer
};
```

### Data Flow

```
CPU                              GPU
 │                                │
 │  Upload palette ──────────────►│  d_palette (constant memory)
 │  Set kernel params ───────────►│  anchor, px_step, max_iters
 │                                │
 │                                │  ┌─ mandelbrot_kernel ─┐
 │                                │  │ 1 thread per pixel   │
 │                                │  │ writes d_pixels      │
 │                                │  └──────────────────────┘
 │                                │
 │  cudaMemcpy D→H ◄────────────│  d_pixels → h_pixels
 │                                │
 │  memcpy to mlx_image           │
 └────────────────────────────────┘
```

### Build Integration

```cmake
# CMakeLists.txt additions
enable_language(CUDA)

add_library(fractal_cuda
    src/gpu/mandelbrot_kernel.cu
    src/gpu/julia_kernel.cu
    src/gpu/newton_kernel.cu
    src/gpu/gpu_renderer.cu
)
set_target_properties(fractal_cuda PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

target_link_libraries(fractol PRIVATE fractal_cuda)
```

---

## Approach B: OpenGL Compute Shaders

### Why This Approach?

MLX42 already creates an OpenGL context (it uses GLFW + OpenGL for rendering).
We can leverage this existing context to dispatch compute shaders **without any
additional dependencies** and with cross-vendor support (NVIDIA, AMD, Intel).

This also creates a natural stepping stone toward WebGPU (Plan 3), since the
shader logic is nearly identical.

### Shader Design

```glsl
// mandelbrot.comp (GLSL 430)
#version 430
layout(local_size_x = 16, local_size_y = 16) in;

layout(std430, binding = 0) buffer PixelBuffer {
    uint pixels[];
};

layout(std430, binding = 1) buffer Palette {
    uint colors[];
};

uniform double anchor_re;
uniform double anchor_im;
uniform double px_step;
uniform uint   max_iters;
uniform uint   width;
uniform uint   height;

void main() {
    uvec2 pos = gl_GlobalInvocationID.xy;
    if (pos.x >= width || pos.y >= height) return;

    dvec2 c = dvec2(
        anchor_re + double(pos.x) * px_step,
        anchor_im - double(pos.y) * px_step
    );
    dvec2 z = dvec2(0.0, 0.0);

    uint iter = 0;
    while (iter < max_iters && dot(z, z) < 4.0) {
        z = dvec2(z.x*z.x - z.y*z.y + c.x,
                  2.0*z.x*z.y + c.y);
        iter++;
    }

    pixels[pos.y * width + pos.x] = colors[iter];
}
```

### Host-Side Integration

The compute shader result is written to an SSBO (Shader Storage Buffer Object),
then read back to the CPU and copied into the MLX image buffer.

Alternatively, the SSBO can be used as a texture source, but since MLX42 manages
its own rendering pipeline, the simplest integration is a GPU→CPU readback.

### Trade-offs: CUDA vs. Compute Shaders

| Aspect | CUDA | Compute Shaders |
|---|---|---|
| **Vendor support** | NVIDIA only | All (NVIDIA, AMD, Intel) |
| **Dependencies** | CUDA Toolkit | None (uses MLX42's GL context) |
| **Double precision** | Native | Requires `GL_ARB_gpu_shader_fp64` |
| **Debugging** | `cuda-gdb`, NSight | RenderDoc, apitrace |
| **WebGPU portability** | None | Shader logic ports almost 1:1 to WGSL |
| **Performance** | Slightly faster (NVIDIA) | Comparable |

> **Recommendation:** Start with **Compute Shaders** (Approach B). It requires no
> extra dependencies, works across GPU vendors, and the shaders translate directly
> to WebGPU's WGSL for Plan 3. Add CUDA as an optional backend later if you need
> maximum NVIDIA performance.

---

## Precision Considerations

| Zoom Depth | Required Precision | Notes |
|---|---|---|
| Up to ~10¹³ | `double` (64-bit) | Standard GPU double precision |
| 10¹³ – 10³⁰ | `double-double` (emulated 128-bit) | 2× doubles, ~4× slower |
| Beyond 10³⁰ | Arbitrary precision | CPU-only, not practical for real-time |

The current C code uses `double` throughout, which is fine for most zoom levels.
GPU `double` is supported but **2× slower** than `float` on most GPUs. For shallow
zooms, `float` is sufficient and 2× faster. A smart approach:

```cpp
// Use float for shallow zooms, double for deep zooms
if (px_step > 1e-7)
    launch_kernel<float>(...);
else
    launch_kernel<double>(...);
```

---

## File Layout (additions to Plan 1)

```
src/
├── gpu/
│   ├── gpu_renderer.hpp / .cpp    # GPU renderer abstraction
│   ├── compute_renderer.hpp / .cpp # OpenGL compute shader backend
│   ├── cuda_renderer.hpp / .cu     # CUDA backend (optional)
│   └── shaders/
│       ├── mandelbrot.comp
│       ├── julia.comp
│       └── newton.comp
```

---

## Migration Checklist

- [ ] Complete Plan 1 (C++ refactor) first — GPU code builds on the C++ types
- [ ] Implement `abs2()` optimization in CPU path (low-hanging fruit)
- [ ] Write `mandelbrot.comp` compute shader
- [ ] Write `ComputeRenderer` host class (compile shader, create SSBOs, dispatch, readback)
- [ ] Integrate with `App` — toggle between CPU and GPU renderer
- [ ] Write `julia.comp` and `newton.comp` compute shaders
- [ ] Profile: measure frames/sec at 500×500, 1080p, and 4K
- [ ] (Optional) Add CUDA backend for NVIDIA-specific optimizations
- [ ] (Optional) Add float/double adaptive precision switching

---

## Risks and Open Questions

| Topic | Notes |
|---|---|
| **MLX42 OpenGL version** | MLX42 uses OpenGL 3.3 core. Compute shaders require **4.3**. We may need to request a 4.3 context from GLFW directly, or patch MLX42's `mlx_init` to request a higher version. |
| **Double precision on GPU** | Not all GPUs support `GL_ARB_gpu_shader_fp64`. Fall back to `float` or CPU rendering if unavailable. |
| **Readback latency** | GPU→CPU `glMapBuffer` / `cudaMemcpy` adds ~0.5ms. For real-time this is fine. For maximum throughput, use double-buffering (render frame N+1 while displaying frame N). |
| **Newton kernel complexity** | Newton requires complex division on the GPU. This is ~4× more ALU-heavy than Mandelbrot. Still massively parallel, but may benefit from shared-memory caching of the palette. |
