// clang-format off

#include "interop/render_resources.hlsli"
#include "interop/constant_buffers.hlsli"

#include "atmosphere/preetham_sky_model.hlsli"

struct VsOutput
{
    float4 position : SV_Position;
    float3 direction : Direction;
};

ConstantBuffer<AtmosphereRenderResources> render_resources : register(b0);

VsOutput vs_main(uint vertex_id : SV_VertexID)  
{
    StructuredBuffer<float3> position_buffer = ResourceDescriptorHeap[render_resources.position_buffer_srv_index];

    ConstantBuffer<SceneBuffer> scene_buffer= ResourceDescriptorHeap[render_resources.scene_buffer_cbv_index];

    VsOutput output;
    output.position = mul(float4(position_buffer[vertex_id], 0.0f), scene_buffer.view_projection_matrix);
    output.position = output.position.xyww;

    output.direction = float3(position_buffer[vertex_id]);

    return output;
}

float4 ps_main(VsOutput input) : SV_Target0
{
    ConstantBuffer<SceneBuffer> scene_buffer= ResourceDescriptorHeap[render_resources.scene_buffer_cbv_index];
    ConstantBuffer<AtmosphereRenderPassBuffer> atmosphere_buffer = ResourceDescriptorHeap[render_resources.atmosphere_buffer_cbv_index];
   
    float4 color = float4(preetham_sky_luminance_and_chromaticity(atmosphere_buffer,  normalize(input.direction), normalize(scene_buffer.sun_direction), atmosphere_buffer.magnitude_multiplier), 1.0f);
    
    return color;
}