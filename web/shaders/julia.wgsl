struct Params {
    anchor_re: f32,
    anchor_im: f32,
    px_step: f32,
    max_iters: u32,
    width: u32,
    height: u32,
    julia_re: f32,
    julia_im: f32,
    color_offset: u32,
    pad1: u32,
    pad2: u32,
    pad3: u32,
};

@group(0) @binding(0) var<uniform> params: Params;
@group(0) @binding(1) var<storage, read> palette: array<u32>;
@group(0) @binding(2) var output_tex: texture_storage_2d<rgba8unorm, write>;

@compute @workgroup_size(16, 16)
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let x = gid.x;
    let y = gid.y;
    if (x >= params.width || y >= params.height) { return; }

    var z_re = params.anchor_re + (f32(x) - f32(params.width) / 2.0) * params.px_step;
    var z_im = params.anchor_im - (f32(y) - f32(params.height) / 2.0) * params.px_step;

    // Use a fixed escape radius of 2.0 (or dynamic if we calculate it in JS)
    let escape_r2 = 4.0;
    var iter: u32 = 0u;

    if (z_re * z_re + z_im * z_im > escape_r2) {
        iter = 0u;
    } else {
        let c_re = params.julia_re;
        let c_im = -params.julia_im; // Mirroring the C++ logic (c_im is negated)
        
        // One iteration before loop to match C++ perfectly:
        let t_re = z_re * z_re - z_im * z_im + c_re;
        let t_im = 2.0 * z_re * z_im + c_im;
        z_re = t_re;
        z_im = t_im;
        iter = 1u;

        loop {
            if (iter >= params.max_iters) { break; }
            let z_re2 = z_re * z_re;
            let z_im2 = z_im * z_im;
            if (z_re2 + z_im2 > escape_r2) { break; }
            z_im = 2.0 * z_re * z_im + c_im;
            z_re = z_re2 - z_im2 + c_re;
            iter = iter + 1u;
        }
    }

    var palette_idx = iter;
    if (iter < params.max_iters) {
        palette_idx = (iter + params.color_offset) % params.max_iters;
    }
    let color = unpack4x8unorm(palette[palette_idx]);
    textureStore(output_tex, vec2<i32>(i32(x), i32(y)), color);
}
