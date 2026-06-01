@group(0) @binding(0) var mySampler: sampler;
@group(0) @binding(1) var myTexture: texture_2d<f32>;

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) uv: vec2<f32>,
};

@vertex
fn vs_main(@builtin(vertex_index) vertexIndex: u32) -> VertexOutput {
    // Generate a fullscreen quad using 3 vertices
    // 0: (-1, -1), 1: (3, -1), 2: (-1, 3)
    let u = f32((vertexIndex << 1u) & 2u);
    let v = f32(vertexIndex & 2u);
    
    var output: VertexOutput;
    output.uv = vec2<f32>(u, v);
    output.position = vec4<f32>(u * 2.0 - 1.0, 1.0 - v * 2.0, 0.0, 1.0);
    return output;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    return textureSample(myTexture, mySampler, in.uv);
}
