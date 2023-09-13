// clang-format off

#include "interop/render_resources.hlsli"
#include "interop/constant_buffers.hlsli"

#include "preetham_sky_model.hlsli"

struct VsOutput
{
    float4 position : SV_Position;
    float2 texture_coord : Texture_Coord;

    // Direction from the origin (0, 0, 0) to screen.
    // Here I am considering screen to be the NDC's back surface (where z = 1.0f).
    float3 direction : DIRECTION;

    // Sun direction is derived from sun angle.
    float3 sun_direction: SUN_DIRECTION;
};

ConstantBuffer<PostProcessCombineRenderResources> render_resources: register(b0);

VsOutput vs_main(uint vertex_id : SV_VertexID)
{
    static const float3 VERTEX_POSITIONS[3] = {float3(-1.0f, 1.0f, 0.0f), float3(3.0f, 1.0f, 0.0f),
                                               float3(-1.0f, -3.0f, 0.0f)};
        
    ConstantBuffer<SceneBuffer> scene_buffer = ResourceDescriptorHeap[render_resources.scene_buffer_cbv_index];

    VsOutput output;
    output.position = float4(VERTEX_POSITIONS[vertex_id], 1.0f);
    output.texture_coord = output.position.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);

    // Get world space coordinates from texture coord.
    // Algorithm taken from here : https://mynameismjp.wordpress.com/2009/03/10/reconstructing-position-from-depth/
    const float4 ndc_coords = float4(output.texture_coord * 2.0f - 1.0f, 1.0f, 1.0f);
    float4 view_space_coords = mul(ndc_coords, scene_buffer.inverse_projection_matrix);
    view_space_coords.xyz /= view_space_coords.w;

    const float3 world_space_coords = mul(view_space_coords, scene_buffer.inverse_view_matrix).xyz;
    output.direction = normalize(world_space_coords.xyz);
    
    // Based on sun angle, construct the sun direction (in world space, similar to direction).
    output.sun_direction = -normalize(float3(0.0f, sin(scene_buffer.sun_angle), cos(scene_buffer.sun_angle)));
    
    return output;
}
    
SamplerState anisotropic_sampler : register(s0);

float4 ps_main(VsOutput input) : SV_Target
{
    Texture2D<float4> render_texture = ResourceDescriptorHeap[render_resources.render_texture_srv_index];
    float4 color = render_texture.Sample(anisotropic_sampler, input.texture_coord);

    // NOTE : Atmosphere computation is put here for now (for testing purposes).
    // Will be removed in future.

    // input.direction is the view direction.
    ConstantBuffer<AtmosphereRenderPassBuffer> atmosphere_renderpass_buffer = ResourceDescriptorHeap[render_resources.atmosphere_buffer_cbv_index];
    color.xyz = preetham_sky_luminance_and_chromaticity(atmosphere_renderpass_buffer, input.direction, input.sun_direction); 

    // Apply gamma correction.
    return float4(color.xyz * 0.05f, 1.0f / 2.2f);
}