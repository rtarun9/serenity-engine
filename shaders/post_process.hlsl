// clang-format off

#include "interop/render_resources.hlsli"
#include "interop/constant_buffers.hlsli"

#include "tone_mappers.hlsli"
#include "utils.hlsli"

struct VsOutput
{
    float4 position : SV_Position;
    float2 texture_coord : Texture_Coord;
};

ConstantBuffer<interop::PostProcessRenderResources> render_resources: register(b0);

VsOutput vs_main(uint vertex_id : SV_VertexID)
{
    static const float3 VERTEX_POSITIONS[3] = {float3(-1.0f, 1.0f, 0.0f), float3(3.0f, 1.0f, 0.0f),
                                               float3(-1.0f, -3.0f, 0.0f)};
        
    VsOutput output;
    output.position = float4(VERTEX_POSITIONS[vertex_id], 1.0f);
    output.texture_coord = output.position.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    
    return output;
}

// Compute random white noise for dithering purposes (i.e intentionally adding some noise to prevent color banding
// by changing the organization of quantization.
// Source : https://github.com/acmarrs/ColorBanding/blob/0b1c37aa9dfcb0fd18f4577eb30db51f41adb239/shaders/ColorBanding.hlsl#L79
float3 generate_white_noise(interop::PostProcessBuffer post_process_buffer, const uint2 position)
{
    // Scaled texture coord (to range (screen_dimension.x, screen_dimension.y)).
    const float2 dimensions = post_process_buffer.screen_dimensions;

    uint seed = post_process_buffer.frame_count * (position.x + (position.y * (uint)dimensions.x));

    float3 rng = float3(0.0f, 0.0f, 0.0f);
    rng.x = random_float_in_range_0_1(seed);
    rng.y = random_float_in_range_0_1(seed);
    rng.z = random_float_in_range_0_1(seed);

    // Shift random values from [0.0, 1.0] to [-0.5, 0.5]
    rng -= 0.5f;

    // Scale noise based on input.
    return rng * post_process_buffer.noise_scale;
}

SamplerState anisotropic_sampler : register(s0);

float4 ps_main(VsOutput input) : SV_Target
{
    Texture2D<float4> render_texture = ResourceDescriptorHeap[render_resources.render_texture_srv_index];
    float4 color = render_texture.Sample(anisotropic_sampler, input.texture_coord);  

    ConstantBuffer<interop::PostProcessBuffer> post_process_buffer = ResourceDescriptorHeap[render_resources.post_process_buffer_cbv_index];

    const float3 noise = generate_white_noise(post_process_buffer, uint2(input.position.xy));

    // Tone mapping.
    color.xyz = aces(color.xyz);

    // Dithering.
    color.xyz += noise;
 
    // Gamma correction.
    color.xyz = pow(color.xyz, 1.0f/2.2f);

    return float4(color.xyz, 1.0f);
}