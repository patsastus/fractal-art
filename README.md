# Fract-ol: High-Performance Fractal Explorer

🌍 **Live Web Demo:** [https://patsastus.github.io/fractal-art/ (you may need to enable webGPU for your browser)](https://patsastus.github.io/fractal-art/)

This project is a real-time fractal explorer supporting the **Mandelbrot**, **Julia**, and **Newton** sets (the last of which isn't strictly fractal by all definitions, but has many fractal-like aspects). 

## The Journey

This project originally started as the **"fract-ol"** assignment from the [42 Network](https://42.fr/en/homepage/) curriculum. The original implementation was written in pure C using the lightweight MiniLibX graphics library, relying entirely on CPU rendering. 

Since then, the codebase has undergone three refactors to push performance, architecture, and accessibility:

### 1. The C++ Refactor
The first step was rebuilding it using modern, object-oriented C++. 
* Replaced the monolithic state machine with a `App`, `Renderer`, and `Fractal` hierarchy.
* Implemented complex number math abstractions.
* Introduced responsive window resizing, and logarithmic shading for the Newton's fractal.

### 2. The CUDA Compute Engine
To achieve smooher rendering and higher iteration depths, the CPU rendering loop was replaced with **NVIDIA CUDA**.
* Kernels were written in `.cu` files to parallelize the fractal math across GPU cores.
* Still uses MiniLibX for the rendering, to keep in touch wiht the history of the project.
* Implemented an on-screen frame-time to benchmark performance gains (you can press G to toggle GPU acceleration on and off).

### 3. The WebGPU Port
To make the project universally accessible, the C++/CUDA logic was ported directly to the browser using webGPU functionality.
* Built using **Vite** and vanilla JavaScript for a lightweight footprint.
* The CUDA compute kernels were translated directly into strict `f32` **WGSL (WebGPU Shading Language)**.
* My first try at an automated CI/CD deployment pipeline via GitHub Actions.

---

## Running Locally

### WebGPU Version (Recommended)
You can run the web version locally to test changes or view it offline. WebGPU is supported by default on Windows/macOS Chrome and Edge. On Linux, you may need to enable unsafe WebGPU flags in your browser (`chrome://flags/#enable-unsafe-webgpu`).

```bash
cd web
npm install
npm run dev
```

### Native C++ / CUDA Version
If you have an NVIDIA GPU and the CUDA Toolkit installed, you can compile the native standalone application. It requires `CMake` and the `MiniLibX` dependencies.

```bash
mkdir build && cd build
cmake ..
make
./fractol mandelbrot
```
*(Usage: `./fractol [mandelbrot|julia|newton]`)*

## Controls (Web & Native)
* **Scroll Wheel:** Zoom in and out (centered perfectly under your mouse pointer)
* **Left Click & Drag:** Pan around the fractal
* **A:** Toggle infinite color animation (Mandelbrot & Julia only)
* **R:** Reset the camera to the default view
* **G:** Toggle GPU acceleration
