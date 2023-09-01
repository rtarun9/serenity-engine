// clang-format off

struct VsOutput
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

VsOutput vs_main(uint vertex_id : SV_VertexID)
{
    static const float4 VERTEX_POSITIONS[3] = 
    {
        float4(-0.5f, -0.5f, 0.0f, 1.0f),
        float4( 0.0f,  0.5f, 0.0f, 1.0f),
        float4( 0.5f, -0.5f, 0.0f, 1.0f),
    };

    static const float4 VERTEX_COLORS[3] = 
    {
        float4(1.0f, 0.0f, 0.0f, 1.0f),
        float4(0.0f, 1.0f, 0.0f, 1.0f),
        float4(0.0f, 0.0f, 1.0f, 1.0f),
    };

    VsOutput output;
    output.position = VERTEX_POSITIONS[vertex_id];
    output.color = VERTEX_COLORS[vertex_id];

    return output;
}

float4 ps_main(VsOutput input) : SV_Target0
{
    return input.color;
}