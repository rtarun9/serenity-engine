// clang-format off

#include "interop/render_resources.hlsli"
#include "interop/constant_buffers.hlsli"

struct VsOutput
{
    float4 position : SV_Position;
};

ConstantBuffer<MeshViewerRenderResources> render_resources : register(b0);

VsOutput vs_main(uint vertex_id : SV_VertexID)  
{
    StructuredBuffer<float3> position_buffer = ResourceDescriptorHeap[render_resources.position_buffer_index];

    ConstantBuffer<TransformBuffer> transform_buffer = ResourceDescriptorHeap[render_resources.transform_buffer_index];

    VsOutput output;
    output.position = mul(float4(position_buffer[vertex_id], 1.0f), transform_buffer.mvp_matrix);

    return output;
}

float4 ps_main(VsOutput input) : SV_Target0
{
    return float4(abs(input.position.x / input.position.w), abs(input.position.y / input.position.w), abs(input.position.z / input.position.w), 1.0f);
}