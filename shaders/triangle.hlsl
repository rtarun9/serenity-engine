// clang-format off

struct RenderResources
{
    uint position_buffer_index;
    uint color_buffer_index;
};

struct VsOutput
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

ConstantBuffer<RenderResources> render_resources : register(b0);

VsOutput vs_main(uint vertex_id : SV_VertexID)  
{
    StructuredBuffer<float3> position_buffer = ResourceDescriptorHeap[render_resources.position_buffer_index];
    StructuredBuffer<float3> color_buffer = ResourceDescriptorHeap[render_resources.color_buffer_index];

    VsOutput output;
    output.position = float4(position_buffer[vertex_id], 1.0f);
    output.color = float4(color_buffer[vertex_id], 1.0f);

    return output;
}

float4 ps_main(VsOutput input) : SV_Target0
{
    return input.color;
}