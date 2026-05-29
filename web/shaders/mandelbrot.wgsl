struct Params {
    anchor_re: f32,
    anchor_im: f32,
    px_step: f32,
    max_iters: u32,
    width: u32,
    height: u32,
    // padding for 16-byte alignment
    pad1: u32,
    pad2: u32,
};

@group(0) @binding(0) var<uniform> params: Params;
@group(0) @binding(1) var<storage, read> palette: array<u32>;
@group(0) @binding(2) var output_tex: texture_storage_2d<rgba8unorm, write>;

@compute @workgroup_size(16, 16)
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let x = gid.x;
    let y = gid.y;
    if (x >= params.width || y >= params.height) { return; }

    let c_re = params.anchor_re + (f32(x) - f32(params.width) / 2.0) * params.px_step;
    let c_im = params.anchor_im - (f32(y) - f32(params.height) / 2.0) * params.px_step;

    var z_re = 0.0;
    var z_im = 0.0;
    var iter: u32 = 0u;

    // Fast check for inside the cardioid or period-2 bulb
    let c2 = c_re * c_re + c_im * c_im;
    if (256.0 * c2 * c2 - 96.0 * c2 + 32.0 * c_re - 3.0 < 0.0) {
        iter = params.max_iters;
    } else if (16.0 * (c2 + 2.0 * c_re + 1.0) - 1.0 < 0.0) {
        iter = params.max_iters;
    } else {
        loop {
            if (iter >= params.max_iters) { break; }
            let z_re2 = z_re * z_re;
            let z_im2 = z_im * z_im;
            if (z_re2 + z_im2 > 4.0) { break; }
            z_im = 2.0 * z_re * z_im + c_im;
            z_re = z_re2 - z_im2 + c_re;
            iter = iter + 1u;
        }
    }

    let color = unpack4x8unorm(palette[iter]);
    textureStore(output_tex, vec2<i32>(i32(x), i32(y)), color);
}
