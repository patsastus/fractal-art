export function buildSawtoothPalette(maxIters) {
  // Returns a Uint32Array of RGBA values (little-endian byte order: ABGR)
  // But WebGPU unpack4x8unorm expects a u32 that unpacks to vec4<f32>.
  // A standard way is to pack as A B G R if we use a specific endianness.
  // Actually, unpack4x8unorm treats the lowest byte as x (R), next as y (G), etc.
  // So: byte 0 = R, byte 1 = G, byte 2 = B, byte 3 = A.
  // We want to pack: R | (G << 8) | (B << 16) | (A << 24)
  const palette = new Uint32Array(maxIters + 1);
  
  for (let i = 0; i < maxIters; i++) {
    // Sawtooth logic from C++
    const factor = (i % (maxIters / 4)) / (maxIters / 4);
    const r = Math.floor(factor * 255);
    const g = Math.floor((i / 2) * 255 / maxIters);
    const b = Math.floor((i / 4) * 255 / maxIters);
    
    // pack as R, G, B, A=255
    palette[i] = (r) | (g << 8) | (b << 16) | (255 << 24);
  }
  // Inside the set = white
  palette[maxIters] = (255) | (255 << 8) | (255 << 16) | (255 << 24);
  return palette;
}

export function buildNewtonPalette(maxIters) {
  const palette = new Uint32Array(maxIters * 3 + 1);
  
  // Base colors for 3 roots: Red, Green, Blue
  const rootColors = [
    [255, 30, 30],
    [30, 255, 30],
    [30, 30, 255]
  ];
  
  for (let root = 0; root < 3; root++) {
    const baseR = rootColors[root][0];
    const baseG = rootColors[root][1];
    const baseB = rootColors[root][2];
    
    for (let i = 0; i < maxIters; i++) {
      // Logarithmic shading logic from C++ fix
      const shade = Math.log(i + 1.0) / Math.log(maxIters + 1.0);
      const intensity = 1.0 - shade; // Darken as iterations increase
      
      const r = Math.floor(baseR * intensity);
      const g = Math.floor(baseG * intensity);
      const b = Math.floor(baseB * intensity);
      
      const idx = root * maxIters + i;
      palette[idx] = (r) | (g << 8) | (b << 16) | (255 << 24);
    }
  }
  
  // Unconverged (black)
  palette[maxIters * 3] = (0) | (0 << 8) | (0 << 16) | (255 << 24);
  return palette;
}
