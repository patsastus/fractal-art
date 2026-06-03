import { Renderer } from "./renderer.js";
import { Controls } from "./controls.js";
import { UI } from "./ui.js";

let renderer, controls, ui;
let stateChanged = true;
let isAnimating = false;

// FPS calculation
let lastTime = performance.now();
let frameCount = 0;

async function init() {
  const canvas = document.getElementById("fractal-canvas");

  renderer = new Renderer(canvas);
  try {
    await renderer.init();
  } catch (e) {
    console.error(e);
    document.getElementById("error-screen").classList.remove("hidden");
    return;
  }

  controls = new Controls(canvas, () => {
    stateChanged = true;
  });

  ui = new UI(
    renderer,
    controls,
    () => {
      isAnimating = !isAnimating;
    },
    () => controls.resetView(),
  );

  // Set initial palette
  ui.updatePalette();

  // Setup resize observer
  const observer = new ResizeObserver((entries) => {
    for (let entry of entries) {
      const width = entry.contentRect.width;
      const height = entry.contentRect.height;
      renderer.resize(width, height);

      // Update canvas logical size
      canvas.width = width;
      canvas.height = height;

      stateChanged = true;
    }
  });
  observer.observe(document.body);

  // Force initial size and center
  renderer.resize(window.innerWidth, window.innerHeight);
  controls.resetView();

  // Start loop
  requestAnimationFrame(frame);
}

function frame(now) {
  // FPS calculation
  frameCount++;
  if (now - lastTime >= 1000) {
    ui.updateFps((frameCount * 1000) / (now - lastTime));
    frameCount = 0;
    lastTime = now;
  }

  if (isAnimating && frameCount % 60 === 0) {
    controls.state.colorOffset = (controls.state.colorOffset + 1) % 100000;
    stateChanged = true;
  }

  if (stateChanged) {
    const gpuTime = renderer.render(controls.state);
    ui.updateStats(gpuTime);
    stateChanged = false;
  }

  requestAnimationFrame(frame);
}

window.addEventListener("DOMContentLoaded", init);
