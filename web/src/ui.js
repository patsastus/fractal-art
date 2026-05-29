import { buildSawtoothPalette, buildNewtonPalette } from './palette.js';

export class UI {
  constructor(renderer, controls, onAnimateToggle, onResetView) {
    this.renderer = renderer;
    this.controls = controls;
    this.onAnimateToggle = onAnimateToggle;
    this.onResetView = onResetView;
    
    // Cache DOM elements
    this.tabs = document.querySelectorAll('.tab-btn');
    this.iterSlider = document.getElementById('iter-slider');
    this.iterVal = document.getElementById('iter-val');
    this.juliaControls = document.getElementById('julia-controls');
    this.juliaRe = document.getElementById('julia-re');
    this.juliaIm = document.getElementById('julia-im');
    
    this.zoomVal = document.getElementById('zoom-val');
    this.fpsVal = document.getElementById('fps-val');
    this.gpuTimeVal = document.getElementById('gpu-time-val');
    
    this.btnAnimate = document.getElementById('btn-animate');
    this.btnReset = document.getElementById('btn-reset');
    
    this.initEvents();
  }
  
  initEvents() {
    this.tabs.forEach(tab => {
      tab.addEventListener('click', () => {
        this.tabs.forEach(t => t.classList.remove('active'));
        tab.classList.add('active');
        const type = tab.dataset.type;
        
        if (type === 'julia') {
          this.juliaControls.classList.remove('hidden');
        } else {
          this.juliaControls.classList.add('hidden');
        }
        
        this.renderer.setFractal(type);
        this.updatePalette();
        this.onResetView();
      });
    });
    
    this.iterSlider.addEventListener('input', (e) => {
      const val = parseInt(e.target.value, 10);
      this.iterVal.textContent = val;
      this.controls.state.maxIters = val;
      this.updatePalette();
      this.controls.onStateChange();
    });
    
    const updateJulia = () => {
      this.controls.state.juliaRe = parseFloat(this.juliaRe.value) || 0.0;
      this.controls.state.juliaIm = parseFloat(this.juliaIm.value) || 0.0;
      this.controls.onStateChange();
    };
    
    this.juliaRe.addEventListener('input', updateJulia);
    this.juliaIm.addEventListener('input', updateJulia);
    
    this.btnAnimate.addEventListener('click', () => this.onAnimateToggle());
    this.btnReset.addEventListener('click', () => this.onResetView());
    
    // Keyboard shortcuts
    window.addEventListener('keydown', (e) => {
      if (e.key.toLowerCase() === 'a') this.onAnimateToggle();
      if (e.key.toLowerCase() === 'r') this.onResetView();
    });
  }
  
  updatePalette() {
    const type = this.renderer.currentFractal;
    const iters = this.controls.state.maxIters;
    
    let palette;
    if (type === 'newton') {
      palette = buildNewtonPalette(iters);
    } else {
      palette = buildSawtoothPalette(iters);
    }
    this.renderer.setPalette(palette);
  }
  
  updateStats(gpuTimeMs) {
    const zoomStr = (0.005 / this.controls.state.pxStep).toFixed(2);
    this.zoomVal.textContent = zoomStr + 'x';
    this.gpuTimeVal.textContent = gpuTimeMs.toFixed(2) + ' ms';
  }
  
  updateFps(fps) {
    this.fpsVal.textContent = Math.round(fps);
  }
  
  showError() {
    document.getElementById('error-screen').classList.remove('hidden');
  }
}
