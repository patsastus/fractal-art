struct Params {
    anchor_re: f32,
    anchor_im: f32,
    px_step: f32,
    max_iters: u32,
    width: u32,
    height: u32,
    pad1: u32,
    pad2: u32,
};

@group(0) @binding(0) var<uniform> params: Params;
@group(0) @binding(1) var<storage, read> palette: array<u32>;
@group(0) @binding(2) var output_tex: texture_storage_2d<rgba8unorm, write>;

fn abs2(re: f32, im: f32) -> f32 {
    return re * re + im * im;
}

fn check_convergence(z_re: f32, z_im: f32) -> u32 {
    let tol2 = 0.0001 * 0.0001;
    let n = 0.86602540378;

    if (abs2(z_re - 1.0, z_im) < tol2) { return 0u; }
    if (abs2(z_re + 0.5, z_im - n) < tol2) { return 1u; }
    if (abs2(z_re + 0.5, z_im + n) < tol2) { return 2u; }
    return 4u;
}

fn get_closest_root(z_re: f32, z_im: f32) -> u32 {
    let n = 0.86602540378;
    var min_dist = 1e10;
    var ret = 0u;

    let d0 = abs2(z_re - 1.0, z_im);
    if (d0 < min_dist) { ret = 0u; min_dist = d0; }

    let d1 = abs2(z_re + 0.5, z_im - n);
    if (d1 < min_dist) { ret = 1u; min_dist = d1; }

    let d2 = abs2(z_re + 0.5, z_im + n);
    if (d2 < min_dist) { return 2u; }

    return ret;
}

@compute @workgroup_size(16, 16)
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let x = gid.x;
    let y = gid.y;
    if (x >= params.width || y >= params.height) { return; }

    var z_re = params.anchor_re + (f32(x) - f32(params.width) / 2.0) * params.px_step;
    var z_im = params.anchor_im - (f32(y) - f32(params.height) / 2.0) * params.px_step;

    var iter: u32 = 0u;
    var root: u32 = 4u; // 4 means unconverged

    loop {
        if (iter >= params.max_iters) { break; }
        
        let z_re2 = z_re * z_re;
        let z_im2 = z_im * z_im;
        
        // t2 = z * z
        let t2_re = z_re2 - z_im2;
        let t2_im = 2.0 * z_re * z_im;
        
        // t3 = t2 * z
        let t3_re = t2_re * z_re - t2_im * z_im;
        let t3_im = t2_re * z_im + t2_im * z_re;
        
        // fz = t3 - 1
        let fz_re = t3_re - 1.0;
        let fz_im = t3_im;
        
        // fpz = 3 * t2
        let fpz_re = 3.0 * t2_re;
        let fpz_im = 3.0 * t2_im;
        
        // step = fz / fpz
        let denom = fpz_re * fpz_re + fpz_im * fpz_im;
        let step_re = (fz_re * fpz_re + fz_im * fpz_im) / denom;
        let step_im = (fz_im * fpz_re - fz_re * fpz_im) / denom;
        
        z_re = z_re - step_re;
        z_im = z_im - step_im;
        
        let check = check_convergence(z_re, z_im);
        if (check != 4u) {
            root = check;
            break;
        }
        iter = iter + 1u;
    }

    if (iter == params.max_iters) {
        root = get_closest_root(z_re, z_im);
    }

    // Palette index: root * max_iters + iter
    var palette_idx = root * params.max_iters + iter;
    if (root == 4u || iter >= params.max_iters) {
        palette_idx = params.max_iters * 3u; // black
    }
    
    let color = unpack4x8unorm(palette[palette_idx]);
    textureStore(output_tex, vec2<i32>(i32(x), i32(y)), color);
}
