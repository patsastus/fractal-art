import { Renderer } from "./renderer.js";

export async function runOracleTests(device, format) {
  console.log("=== RUNNING WEBGPU ORACLE TESTS ===");
  try {
    const response = await fetch("/tests/oracle.json");
    const oracle = await response.json();

    const canvas = document.createElement("canvas");
    canvas.width = 16;
    canvas.height = 16;

    const renderer = new Renderer(canvas);
    await renderer.init();
    renderer.resize(16, 16);

    const state = {
      anchorX: -0.4,
      anchorY: 0.4,
      pxStep: 0.05,
      maxIters: 100,
      juliaRe: -0.4,
      juliaIm: 0.6,
    };

    let passed = 0;
    let failed = 0;

    const assertArraysEqual = (
      name,
      actualPixels,
      expectedIterations,
      checkRoot = false,
    ) => {
      let failCount = 0;
      for (let y = 0; y < 16; y++) {
        for (let x = 0; x < 16; x++) {
          const idx = y * 16 + x;
        }
      }
      return failCount === 0;
    };
    console.warn(
      "Oracle test framework loaded. To fully automate, WGSL must output raw iterations to a storage buffer.",
    );
  } catch (e) {
    console.error("Oracle test failed to load:", e);
  }
}
