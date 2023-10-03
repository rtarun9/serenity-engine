// clang-format off

#include "interop/render_resources.hlsli"
#include "interop/constant_buffers.hlsli"

struct VsOutput
{
    float4 position : SV_Position;
    float3 color : COLOR;
};

ConstantBuffer<interop::LightRenderResources> render_resources : register(b0);

VsOutput vs_main(uint vertex_id : SV_VertexID, uint instance_id: SV_InstanceID)
{
    StructuredBuffer<float3> position_buffer = ResourceDescriptorHeap[render_resources.light_cube_position_buffer_srv_index];
    ConstantBuffer<interop::SceneBuffer> scene_buffer= ResourceDescriptorHeap[render_resources.scene_buffer_cbv_index];
    ConstantBuffer<interop::LightBuffer> light_buffer= ResourceDescriptorHeap[render_resources.light_buffer_cbv_index];

    VsOutput output;
    output.position = mul(float4(position_buffer[vertex_id], 1.0f), mul(light_buffer.model_matrix[instance_id], scene_buffer.view_projection_matrix));

    // For instanced rendering, instance_id = x implies light index is x + 1 (since the directional light does not have a visualization cube to be rendered).
    const uint light_index = instance_id + 1u;

    output.color = light_buffer.lights[light_index].color * light_buffer.lights[light_index].intensity;

    return output;
}

float4 ps_main(VsOutput input) : SV_Target0
{
    return float4(input.color, 1.0f);
}