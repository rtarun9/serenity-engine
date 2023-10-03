// clang-format off

#include "interop/render_resources.hlsli"
#include "interop/constant_buffers.hlsli"

struct VsOutput
{
    float4 position : SV_Position;
    float2 texture_coord : Texture_Coord;
};

ConstantBuffer<interop::PostProcessCombineRenderResources> render_resources: register(b0);

VsOutput vs_main(uint vertex_id : SV_VertexID)
{
    static const float3 VERTEX_POSITIONS[3] = {float3(-1.0f, 1.0f, 0.0f), float3(3.0f, 1.0f, 0.0f),
                                               float3(-1.0f, -3.0f, 0.0f)};
        
    VsOutput output;
    output.position = float4(VERTEX_POSITIONS[vertex_id], 1.0f);
    output.texture_coord = output.position.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    
    return output;
}

// Aces Narkowicz approximation (to convert HDR to LDR).
// Formula from : https://64.github.io/tonemapping/
float3 aces_approximation(float3 hdr_color)
{
    hdr_color *= 0.6f;

    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;

    return clamp((hdr_color * (a * hdr_color + b)) / (hdr_color * (c * hdr_color + d) + e), 0.0f, 1.0f);
}

SamplerState anisotropic_sampler : register(s0);

float4 ps_main(VsOutput input) : SV_Target
{
    Texture2D<float4> render_texture = ResourceDescriptorHeap[render_resources.render_texture_srv_index];
    float4 color = render_texture.Sample(anisotropic_sampler, input.texture_coord);  
    
    // Tone mapping.
    color.xyz = aces_approximation(color.xyz);
    
    // Apply gamma correction.
    return float4(pow(color.xyz, 1.0f / 2.2f), 1.0f);
}