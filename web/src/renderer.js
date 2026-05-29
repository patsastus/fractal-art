import mandelbrotWgsl from '../shaders/mandelbrot.wgsl?raw';
import juliaWgsl from '../shaders/julia.wgsl?raw';
import newtonWgsl from '../shaders/newton.wgsl?raw';
import renderWgsl from '../shaders/render.wgsl?raw';

export class Renderer {
  constructor(canvas) {
    this.canvas = canvas;
    this.device = null;
    this.context = null;
    this.format = null;
    
    this.pipelines = {};
    this.renderPipeline = null;
    
    this.uniformBuffer = null;
    this.paletteBuffer = null;
    this.storageTexture = null;
    this.bindGroup = null;
    this.renderBindGroup = null;
    
    this.currentFractal = 'mandelbrot';
    
    // Uniform data array: [anchor_re, anchor_im, px_step, max_iters, width, height, julia_re, julia_im]
    this.uniformData = new Float32Array(8);
  }

  async init() {
    if (!navigator.gpu) {
      throw new Error("WebGPU not supported on this browser.");
    }

    const adapter = await navigator.gpu.requestAdapter();
    if (!adapter) {
      throw new Error("No appropriate GPU adapter found.");
    }

    this.device = await adapter.requestDevice();
    this.context = this.canvas.getContext('webgpu');
    this.format = navigator.gpu.getPreferredCanvasFormat();

    this.context.configure({
      device: this.device,
      format: this.format,
      alphaMode: 'premultiplied',
    });

    await this.initPipelines();
    this.initBuffers();
  }

  async initPipelines() {
    const createCompute = (code) => {
      const module = this.device.createShaderModule({ code });
      return this.device.createComputePipeline({
        layout: 'auto',
        compute: { module, entryPoint: 'main' }
      });
    };

    this.pipelines.mandelbrot = createCompute(mandelbrotWgsl);
    this.pipelines.julia = createCompute(juliaWgsl);
    this.pipelines.newton = createCompute(newtonWgsl);

    const renderModule = this.device.createShaderModule({ code: renderWgsl });
    this.renderPipeline = this.device.createRenderPipeline({
      layout: 'auto',
      vertex: { module: renderModule, entryPoint: 'vs_main' },
      fragment: {
        module: renderModule,
        entryPoint: 'fs_main',
        targets: [{ format: this.format }]
      },
      primitive: { topology: 'triangle-list' }
    });
  }

  initBuffers() {
    // 8 floats = 32 bytes
    this.uniformBuffer = this.device.createBuffer({
      size: 32,
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });

    // Palette max size: newton needs 2000 * 3 + 1 = 6001 u32s. 
    // Allocate 8192 * 4 = 32768 bytes just to be safe.
    this.paletteBuffer = this.device.createBuffer({
      size: 32768,
      usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_DST,
    });
    
    this.sampler = this.device.createSampler({
      magFilter: 'linear',
      minFilter: 'linear',
    });
  }

  resize(width, height) {
    this.canvas.width = width;
    this.canvas.height = height;

    if (this.storageTexture) {
      this.storageTexture.destroy();
    }

    this.storageTexture = this.device.createTexture({
      size: [width, height, 1],
      format: 'rgba8unorm',
      usage: GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING,
    });

    // Create bind groups for the new texture
    this.updateBindGroups();
  }

  updateBindGroups() {
    if (!this.storageTexture) return;

    this.bindGroup = this.device.createBindGroup({
      layout: this.pipelines[this.currentFractal].getBindGroupLayout(0),
      entries: [
        { binding: 0, resource: { buffer: this.uniformBuffer } },
        { binding: 1, resource: { buffer: this.paletteBuffer } },
        { binding: 2, resource: this.storageTexture.createView() }
      ]
    });

    this.renderBindGroup = this.device.createBindGroup({
      layout: this.renderPipeline.getBindGroupLayout(0),
      entries: [
        { binding: 0, resource: this.sampler },
        { binding: 1, resource: this.storageTexture.createView() }
      ]
    });
  }

  setFractal(type) {
    if (this.pipelines[type]) {
      this.currentFractal = type;
      this.updateBindGroups();
    }
  }

  setPalette(paletteArray) {
    this.device.queue.writeBuffer(this.paletteBuffer, 0, paletteArray);
  }

  setUniforms(state) {
    this.uniformData[0] = state.anchorX;
    this.uniformData[1] = state.anchorY;
    this.uniformData[2] = state.pxStep;
    
    // JS typed array views to write u32 into the float array buffer safely
    const u32View = new Uint32Array(this.uniformData.buffer);
    u32View[3] = state.maxIters;
    u32View[4] = this.canvas.width;
    u32View[5] = this.canvas.height;
    
    this.uniformData[6] = state.juliaRe || 0.0;
    this.uniformData[7] = state.juliaIm || 0.0;

    this.device.queue.writeBuffer(this.uniformBuffer, 0, this.uniformData);
  }

  render(state) {
    if (!this.device || !this.storageTexture) return 0;
    
    const startTime = performance.now();
    this.setUniforms(state);

    const encoder = this.device.createCommandEncoder();

    // 1. Compute Pass
    const computePass = encoder.beginComputePass();
    computePass.setPipeline(this.pipelines[this.currentFractal]);
    computePass.setBindGroup(0, this.bindGroup);
    computePass.dispatchWorkgroups(
      Math.ceil(this.canvas.width / 16),
      Math.ceil(this.canvas.height / 16)
    );
    computePass.end();

    // 2. Render Pass
    const renderPass = encoder.beginRenderPass({
      colorAttachments: [{
        view: this.context.getCurrentTexture().createView(),
        clearValue: { r: 0, g: 0, b: 0, a: 1 },
        loadOp: 'clear',
        storeOp: 'store',
      }]
    });
    renderPass.setPipeline(this.renderPipeline);
    renderPass.setBindGroup(0, this.renderBindGroup);
    renderPass.draw(3); // fullscreen triangle
    renderPass.end();

    this.device.queue.submit([encoder.finish()]);
    
    return performance.now() - startTime;
  }
  
  // Method used for headless testing to read the texture buffer back to CPU
  async readbackStorageTexture() {
    const width = this.canvas.width;
    const height = this.canvas.height;
    
    // Create a readback buffer
    const size = width * height * 4; // rgba8unorm is 4 bytes per pixel
    const readbackBuffer = this.device.createBuffer({
      size: size,
      usage: GPUBufferUsage.COPY_DST | GPUBufferUsage.MAP_READ,
    });
    
    const encoder = this.device.createCommandEncoder();
    encoder.copyTextureToBuffer(
      { texture: this.storageTexture },
      { buffer: readbackBuffer, bytesPerRow: width * 4 },
      [width, height, 1]
    );
    this.device.queue.submit([encoder.finish()]);
    
    await readbackBuffer.mapAsync(GPUMapMode.READ);
    const data = new Uint8Array(readbackBuffer.getMappedRange());
    // Copy so we can unmap
    const result = new Uint8Array(data);
    readbackBuffer.unmap();
    
    return result;
  }
}
