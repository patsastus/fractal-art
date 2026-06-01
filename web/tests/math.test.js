import { describe, it, expect } from 'vitest';
import { buildSawtoothPalette, buildNewtonPalette } from '../src/palette.js';

describe('Palette Generation', () => {
  it('buildSawtoothPalette handles max iterations', () => {
    const maxIters = 100;
    const palette = buildSawtoothPalette(maxIters);
    
    // Length should be maxIters + 1
    expect(palette.length).toBe(101);
    
    // Inside the set (last index) should be white (0xFFFFFFFF in ABGR format)
    expect(palette[100]).toBe(4294967295); // (255) | (255<<8) | (255<<16) | (255<<24) = 0xFFFFFFFF
    
    // 0 iterations
    expect(palette[0] & 0xFF).toBe(0); // r = 0
  });

  it('buildNewtonPalette uses logarithmic shading', () => {
    const maxIters = 100;
    const palette = buildNewtonPalette(maxIters);
    
    // Length: 3 roots * maxIters + 1 (unconverged)
    expect(palette.length).toBe(301);
    
    // Root 0, Iteration 0 (intensity 1.0)
    // base = [255, 30, 30]
    // shade = log(1)/log(101) = 0
    // intensity = 1.0
    const color0 = palette[0];
    const r0 = color0 & 0xFF;
    const g0 = (color0 >> 8) & 0xFF;
    const b0 = (color0 >> 16) & 0xFF;
    expect(r0).toBe(255);
    expect(g0).toBe(30);
    expect(b0).toBe(30);
    
    // Unconverged (black)
    expect(palette[300]).toBe((255 << 24) >>> 0); // alpha=255, others=0
  });
});
