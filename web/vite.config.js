import { defineConfig } from 'vite';

export default defineConfig({
  // Base path for github pages. If your repo is named fractol-refactor,
  // this should be /fractol-refactor/. Adjust if necessary.
  base: './', 
  
  // Custom plugin to load .wgsl files as raw strings
  plugins: [
    {
      name: 'wgsl-raw-loader',
      transform(code, id) {
        if (id.endsWith('.wgsl')) {
          // Return the shader code as a default exported string
          return {
            code: `export default ${JSON.stringify(code)};`,
            map: { mappings: '' }
          };
        }
      }
    }
  ]
});
