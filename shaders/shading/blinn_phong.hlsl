// clang-format off

#include "interop/render_resources.hlsli"
#include "interop/constant_buffers.hlsli"

struct VsOutput
{
    float4 position : SV_Position;
    float2 texture_coord : TEXTURE_COORD;
};

ConstantBuffer<MeshViewerRenderResources> render_resources : register(b0);

VsOutput vs_main(uint vertex_id : SV_VertexID)  
{
    StructuredBuffer<float3> position_buffer = ResourceDescriptorHeap[render_resources.position_buffer_srv_index];
    StructuredBuffer<float2> texture_coord_buffer = ResourceDescriptorHeap[render_resources.texture_coord_buffer_srv_index];

    ConstantBuffer<TransformBuffer> transform_buffer = ResourceDescriptorHeap[render_resources.transform_buffer_cbv_index];
    ConstantBuffer<SceneBuffer> scene_buffer= ResourceDescriptorHeap[render_resources.scene_buffer_cbv_index];

    VsOutput output;
    output.position = mul(float4(position_buffer[vertex_id], 1.0f), mul(transform_buffer.model_matrix, scene_buffer.view_projection_matrix));
    output.texture_coord = texture_coord_buffer[vertex_id];

    return output;
}

SamplerState anisotropic_sampler : register(s0);

float4 ps_main(VsOutput input) : SV_Target0
{
    ConstantBuffer<MaterialBuffer> material_buffer = ResourceDescriptorHeap[render_resources.material_buffer_cbv_index];
    float4 color = material_buffer.base_color;

    if (material_buffer.albedo_texture_srv_index != INVALID_INDEX_U32)
    {
        Texture2D<float4> albedo_texture = ResourceDescriptorHeap[material_buffer.albedo_texture_srv_index];
        color = albedo_texture.Sample(anisotropic_sampler, input.texture_coord);
    }

    return color;
}