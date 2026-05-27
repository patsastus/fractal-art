# Plan 3: WebGPU Port for Browser-Based Access

## Overview

Port the fractal renderer to run entirely in a web browser using **WebGPU** for
GPU-accelerated computation and rendering. This replaces the MLX42/GLFW desktop
windowing with an HTML Canvas, and the OpenGL compute shaders (from Plan 2) with
**WGSL** (WebGPU Shading Language) compute shaders.

The end result: a URL that anyone can visit to explore Mandelbrot, Julia, and
Newton fractals with real-time zoom and pan — no installation required.

---

## Architecture Overview

```
┌─────────────────────────────────────────────────┐
│                   Browser                        │
│                                                  │
│  ┌──────────────┐    ┌──────────────────────┐   │
│  │  HTML / CSS   │    │  JavaScript (App)     │   │
│  │  Canvas       │◄──│  - Input handling      │   │
│  │  UI controls  │    │  - Zoom/pan state      │   │
│  └──────────────┘    │  - Palette generation   │   │
│                       │  - WebGPU orchestration│   │
│                       └──────────┬─────────────┘   │
│                                  │                  │
│                       ┌──────────▼─────────────┐   │
│                       │  WebGPU Pipeline        │   │
│                       │  - Compute shader       │   │
│                       │    (fractal iteration)  │   │
│                       │  - Render pass          │   │
│                       │    (fullscreen quad)    │   │
│                       └────────────────────────┘   │
└─────────────────────────────────────────────────┘
```

**Key difference from Plans 1 & 2:** No CPU readback. The compute shader writes
directly to a storage texture, and the render shader samples it to the canvas.
Everything stays on the GPU.

---

## WebGPU Compute Shaders (WGSL)

### Mandelbrot Shader

```wgsl
// mandelbrot.wgsl
struct Params {
    anchor_re:  f64,
    anchor_im:  f64,
    px_step:    f64,
    max_iters:  u32,
    width:      u32,
    height:     u32,
};

@group(0) @binding(0) var<uniform> params: Params;
@group(0) @binding(1) var<storage, read> palette: array<u32>;
@group(0) @binding(2) var output_tex: texture_storage_2d<rgba8unorm, write>;

@compute @workgroup_size(16, 16)
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let x = gid.x;
    let y = gid.y;
    if (x >= params.width || y >= params.height) { return; }

    let c_re = params.anchor_re + f64(x) * params.px_step;
    let c_im = params.anchor_im - f64(y) * params.px_step;

    var z_re = 0.0;
    var z_im = 0.0;
    var iter: u32 = 0u;

    loop {
        if (iter >= params.max_iters) { break; }
        let z_re2 = z_re * z_re;
        let z_im2 = z_im * z_im;
        if (z_re2 + z_im2 > 4.0) { break; }
        z_im = 2.0 * z_re * z_im + c_im;
        z_re = z_re2 - z_im2 + c_re;
        iter = iter + 1u;
    }

    let color = unpack4x8unorm(palette[iter]);
    textureStore(output_tex, vec2<i32>(i32(x), i32(y)), color);
}
```

> **Note on f64:** WebGPU `f64` support requires the `"shader-f64"` feature,
> which is not yet universally available. A practical fallback is to use `f32`
> for shallow zooms and switch to an emulated double-float technique for deep
> zooms (see Precision section below).

### Julia and Newton shaders

These follow the same pattern — only the inner iteration loop changes. The Julia
shader takes an additional uniform `c_re, c_im` for the Julia parameter. The
Newton shader computes `f(z)/f'(z)` and checks convergence against 3 solutions.

---

## JavaScript Application Layer

### Core Modules

```
web/
├── index.html           # Canvas + UI controls
├── style.css            # Styling for controls overlay
├── src/
│   ├── main.js          # Entry point, initializes WebGPU
│   ├── gpu.js           # WebGPU device, pipeline, buffer management
│   ├── fractals.js      # Fractal-specific params, shader selection
│   ├── palette.js       # Color palette generation (sawtooth, Newton)
│   ├── controls.js      # Mouse/touch/keyboard input → zoom/pan state
│   └── ui.js            # Fractal selector, iteration slider, info overlay
└── shaders/
    ├── mandelbrot.wgsl
    ├── julia.wgsl
    ├── newton.wgsl
    └── fullscreen.wgsl  # Render pass: samples storage texture to canvas
```

### WebGPU Initialization (`gpu.js`)

```javascript
export async function initGPU(canvas) {
    const adapter = await navigator.gpu.requestAdapter();
    const device  = await adapter.requestDevice({
        requiredFeatures: ['shader-f64'],  // if supported
    });
    const context = canvas.getContext('webgpu');
    const format  = navigator.gpu.getPreferredCanvasFormat();
    context.configure({ device, format, alphaMode: 'premultiplied' });

    return { adapter, device, context, format };
}
```

### Render Loop (`main.js`)

```javascript
function frame() {
    if (stateChanged) {
        // Update uniform buffer with current anchor, px_step, max_iters
        device.queue.writeBuffer(uniformBuffer, 0, uniformData);

        // Dispatch compute shader
        const encoder = device.createCommandEncoder();
        const pass = encoder.beginComputePass();
        pass.setPipeline(computePipeline);
        pass.setBindGroup(0, bindGroup);
        pass.dispatchWorkgroups(
            Math.ceil(width / 16),
            Math.ceil(height / 16)
        );
        pass.end();

        // Render pass: draw fullscreen quad sampling the storage texture
        const renderPass = encoder.beginRenderPass({ /* ... */ });
        renderPass.setPipeline(renderPipeline);
        renderPass.setBindGroup(0, renderBindGroup);
        renderPass.draw(6);  // fullscreen quad (2 triangles)
        renderPass.end();

        device.queue.submit([encoder.finish()]);
        stateChanged = false;
    }
    requestAnimationFrame(frame);
}
```

---

## User Interface Design

### Controls Overlay

A minimal, semi-transparent overlay on top of the fractal canvas:

```
┌─────────────────────────────────────────────────────────┐
│  ┌─────────────────────────────────────────────────┐    │
│  │           [Mandelbrot] [Julia] [Newton]          │    │
│  └─────────────────────────────────────────────────┘    │
│                                                          │
│                                                          │
│                    (fractal canvas)                       │
│                                                          │
│                                                          │
│  ┌─────────────────────────────────────────────────┐    │
│  │  Iterations: [====|============] 100             │    │
│  │  Julia c:  re [-0.4] im [0.6]     [Animate ▶]  │    │
│  │  Zoom: 1.00x  │  FPS: 60  │  [Reset View]      │    │
│  └─────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────┘
```

### Interaction Model

| Input | Action |
|---|---|
| Scroll wheel | Zoom in/out (centered on cursor) |
| Click + drag | Pan |
| Pinch (touch) | Zoom on mobile |
| Arrow keys | Pan (discrete steps) |
| `R` | Reset view |
| Fractal selector | Switch between Mandelbrot / Julia / Newton |
| Iteration slider | Adjust max iterations (1–2000) |
| Julia inputs | Set c parameter (only visible in Julia mode) |
| Animate toggle | Cycle through color palette |

### Responsive Design

- Canvas fills viewport (`width: 100vw; height: 100vh`)
- Controls overlay uses `position: fixed` with subtle backdrop blur
- Touch events mapped for mobile: pinch-zoom, drag-pan
- Minimum viable mobile experience at 360px width

---

## Precision Strategy for Deep Zooms

Since `f64` support in WebGPU is not yet universal, we need a fallback:

### Emulated Double-Float (Dekker's Algorithm)

Represent each `f64` as a pair of `f32` values `(hi, lo)` where the true value
is `hi + lo`. This gives ~48 bits of mantissa (vs. 23 for single `f32`).

```wgsl
struct df64 {
    hi: f32,
    lo: f32,
};

fn df64_add(a: df64, b: df64) -> df64 {
    let s = a.hi + b.hi;
    let v = s - a.hi;
    let t = (b.hi - v) + (a.hi - (s - v)) + a.lo + b.lo;
    return df64(s + t, t - (s + t - s));
}

fn df64_mul(a: df64, b: df64) -> df64 {
    let p = a.hi * b.hi;
    let e = fma(a.hi, b.hi, -p) + a.hi * b.lo + a.lo * b.hi;
    return df64(p + e, e - (p + e - p));
}
```

**Performance impact:** ~3–4× slower than native `f32`, but still massively
faster than CPU. Use `f32` when `px_step > 1e-5`, switch to `df64` below that.

---

## Deployment Options

### Option A: Static Site (Recommended for simplicity)

No backend needed. The entire app is static HTML/JS/WGSL files.

```bash
# Serve locally
npx serve web/

# Or deploy to any static host:
# - GitHub Pages
# - Netlify
# - Vercel
# - Cloudflare Pages
```

### Option B: Bundled with Vite

For a more polished dev experience with hot-reload and bundling:

```bash
npx -y create-vite@latest ./ --template vanilla
npm install
npm run dev
```

Vite handles:
- WGSL files as raw imports (`import shaderCode from './shaders/mandelbrot.wgsl?raw'`)
- Hot module replacement during development
- Optimized production build with minification

### Browser Support

| Browser | WebGPU Status |
|---|---|
| Chrome 113+ | ✅ Stable |
| Edge 113+ | ✅ Stable |
| Firefox 140+ | ✅ Stable |
| Safari 18+ | ✅ Stable |
| Mobile Chrome | ✅ (Android 13+) |
| Mobile Safari | ⚠️ Limited (iOS 18+) |

> **Fallback:** For browsers without WebGPU, display a message with a link to
> a supported browser rather than implementing a Canvas2D CPU fallback (which
> would be too slow to be useful).

---

## Migration Checklist

- [ ] Set up project scaffolding (`index.html`, Vite or static)
- [ ] Implement WebGPU initialization and canvas setup
- [ ] Write `mandelbrot.wgsl` compute shader
- [ ] Write fullscreen quad render shader
- [ ] Implement zoom/pan state management and input handlers
- [ ] Generate color palettes in JS (port `make_colors` and `make_colors_newton`)
- [ ] Write `julia.wgsl` with parameter uniform
- [ ] Write `newton.wgsl` with convergence checking
- [ ] Build UI overlay (fractal selector, iteration slider, Julia inputs)
- [ ] Implement color cycling animation
- [ ] Add touch/mobile support
- [ ] Implement emulated double-float for deep zoom fallback
- [ ] Test across browsers (Chrome, Firefox, Safari, Edge)
- [ ] Deploy to a static hosting service
- [ ] (Optional) Add URL state for shareable zoom coordinates

---

## Risks and Open Questions

| Topic | Notes |
|---|---|
| **f64 support** | `shader-f64` is an optional WebGPU feature. If not available, max useful zoom depth is ~10⁵ with `f32`, or ~10¹⁰ with emulated double-float. For a web demo this is usually sufficient. |
| **Mobile performance** | Mobile GPUs are weaker. Reduce resolution or max_iters dynamically based on frame time. Target 30fps minimum. |
| **WGSL maturity** | WGSL is still evolving. Some advanced features (like `textureStore` with certain formats) may need workarounds on specific browsers. Test early. |
| **Relationship to Plan 2** | If the OpenGL compute shader path (Plan 2, Approach B) is implemented first, the WGSL shaders will be near-direct translations. GLSL → WGSL is mostly syntactic. If CUDA (Plan 2, Approach A) is the only GPU path, the WGSL shaders need to be written from scratch (but the logic is identical). |
| **Hosting costs** | Static hosting is free on GitHub Pages, Netlify, and Cloudflare Pages. No server costs unless you add analytics or user accounts. |
| **SEO / discoverability** | Add proper `<meta>` tags, Open Graph images (a screenshot of the Mandelbrot set), and a descriptive `<title>` for social sharing. |
