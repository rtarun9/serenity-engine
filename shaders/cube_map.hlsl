// clang-format off

#include "interop/render_resources.hlsli"
#include "interop/constant_buffers.hlsli"

struct VsOutput
{
    float4 position : SV_Position;
    float3 direction : Direction;
};

ConstantBuffer<interop::CubeMapRenderResources> render_resources : register(b0);

VsOutput vs_main(uint vertex_id : SV_VertexID)      
{
    StructuredBuffer<float3> position_buffer = ResourceDescriptorHeap[render_resources.position_buffer_srv_index];

    ConstantBuffer<interop::SceneBuffer> scene_buffer= ResourceDescriptorHeap[render_resources.scene_buffer_cbv_index];

    VsOutput output;
    output.position = mul(float4(position_buffer[vertex_id], 0.0f), scene_buffer.view_projection_matrix);
    output.position = output.position.xyww;

    output.direction = float3(position_buffer[vertex_id]);

    return output;
}

SamplerState linear_wrap_sampler : register(s1);

float4 ps_main(VsOutput input) : SV_Target0
{
    TextureCube<float4> atmosphere_texture_cube = ResourceDescriptorHeap[render_resources.texture_srv_index];

    return atmosphere_texture_cube.Sample(linear_wrap_sampler, normalize(input.direction));
}