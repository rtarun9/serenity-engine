// clang-format off

#include "interop/render_resources.hlsli"
#include "interop/constant_buffers.hlsli"

#include "preetham_sky_model.hlsli"

struct VsOutput
{
    float4 position : SV_Position;
    float2 texture_coord : Texture_Coord;
};

ConstantBuffer<PostProcessCombineRenderResources> render_resources: register(b0);

VsOutput vs_main(uint vertex_id : SV_VertexID)
{
    static const float3 VERTEX_POSITIONS[3] = {float3(-1.0f, 1.0f, 0.0f), float3(3.0f, 1.0f, 0.0f),
                                               float3(-1.0f, -3.0f, 0.0f)};

    VsOutput output;
    output.position = float4(VERTEX_POSITIONS[vertex_id], 1.0f);
    output.texture_coord = output.position.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    return output;
}

SamplerState anisotropic_sampler : register(s0);

float4 ps_main(VsOutput input) : SV_Target
{
    Texture2D<float4> render_texture = ResourceDescriptorHeap[render_resources.render_texture_srv_index];
    float4 color = render_texture.Sample(anisotropic_sampler, input.texture_coord);

    // NOTE : Atmosphere computation is put here for now (for testing purposes).
    // Will be removed in future.
    
    ConstantBuffer<AtmosphereRenderPassBuffer> atmosphere_renderpass_buffer = ResourceDescriptorHeap[render_resources.atmosphere_buffer_cbv_index];

    // Apply gamma correction.
    return pow(color, 1/2.2f);
}