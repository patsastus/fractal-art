export class Controls {
  constructor(canvas, onStateChange) {
    this.canvas = canvas;
    this.onStateChange = onStateChange;

    this.state = {
      anchorX: 0.0,
      anchorY: 0.0,
      pxStep: 0.005,
      maxIters: 100,
      juliaRe: -0.4,
      juliaIm: 0.6
    };

    this.isDragging = false;
    this.lastMouseX = 0;
    this.lastMouseY = 0;
    
    this.keys = new Set();
    
    this.initEvents();
  }

  resetView(escapeRadius = 2.0) {
    const minDim = Math.min(this.canvas.width, this.canvas.height);
    // Same zoom scaling logic as C++ app.cpp
    this.state.pxStep = (escapeRadius * 3.0) / minDim;
    this.state.anchorX = 0.0;
    this.state.anchorY = 0.0;
    this.onStateChange();
  }

  initEvents() {
    this.canvas.addEventListener('mousedown', (e) => {
      this.isDragging = true;
      this.lastMouseX = e.clientX;
      this.lastMouseY = e.clientY;
    });

    window.addEventListener('mouseup', () => {
      this.isDragging = false;
    });

    window.addEventListener('mousemove', (e) => {
      if (this.isDragging) {
        const dx = e.clientX - this.lastMouseX;
        const dy = e.clientY - this.lastMouseY;
        
        // Panning: shift the anchor by the pixel delta
        this.state.anchorX -= dx * this.state.pxStep;
        this.state.anchorY += dy * this.state.pxStep; // y-axis is inverted
        
        this.lastMouseX = e.clientX;
        this.lastMouseY = e.clientY;
        this.onStateChange();
      }
    });

    this.canvas.addEventListener('wheel', (e) => {
      e.preventDefault();
      
      const zoomFactor = 1.1;
      const delta = e.deltaY;
      
      // Calculate mouse position relative to canvas
      const rect = this.canvas.getBoundingClientRect();
      const x = e.clientX - rect.left;
      const y = e.clientY - rect.top;
      
      // Shift anchor to mouse position
      const mouseRe = this.state.anchorX + (x - this.canvas.width / 2.0) * this.state.pxStep;
      const mouseIm = this.state.anchorY - (y - this.canvas.height / 2.0) * this.state.pxStep;
      
      this.state.anchorX = mouseRe;
      this.state.anchorY = mouseIm;
      
      if (delta > 0 && this.state.pxStep < 1000.0) {
        this.state.pxStep *= zoomFactor;
      } else if (delta < 0 && this.state.pxStep > 1e-15) {
        this.state.pxStep *= (2.0 - zoomFactor);
      }
      
      this.onStateChange();
    }, { passive: false });
    
    window.addEventListener('keydown', (e) => {
      this.keys.add(e.key);
      this.handleKeys();
    });
    
    window.addEventListener('keyup', (e) => {
      this.keys.delete(e.key);
    });
  }
  
  handleKeys() {
    const panStep = (this.canvas.width / 10) * this.state.pxStep;
    let changed = false;
    
    if (this.keys.has('ArrowLeft')) { this.state.anchorX -= panStep; changed = true; }
    if (this.keys.has('ArrowRight')) { this.state.anchorX += panStep; changed = true; }
    if (this.keys.has('ArrowUp')) { this.state.anchorY += panStep; changed = true; }
    if (this.keys.has('ArrowDown')) { this.state.anchorY -= panStep; changed = true; }
    
    if (changed) {
      this.onStateChange();
    }
  }
}
