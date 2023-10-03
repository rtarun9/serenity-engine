// clang-format off

#include "interop/render_resources.hlsli"
#include "interop/constant_buffers.hlsli"

struct VsOutput
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

ConstantBuffer<interop::TriangleRenderResources> render_resources : register(b0);

VsOutput vs_main(uint vertex_id : SV_VertexID)  
{
    StructuredBuffer<float3> position_buffer = ResourceDescriptorHeap[render_resources.position_buffer_index];
    StructuredBuffer<float3> color_buffer = ResourceDescriptorHeap[render_resources.color_buffer_index];

    ConstantBuffer<interop::TransformBuffer> transform_buffer = ResourceDescriptorHeap[render_resources.transform_buffer_index];

    VsOutput output;
    output.position = mul(float4(position_buffer[vertex_id], 1.0f), transform_buffer.mvp_matrix);
    output.color = float4(color_buffer[vertex_id], 1.0f);

    return output;
}

float4 ps_main(VsOutput input) : SV_Target0
{
    return input.color;
}