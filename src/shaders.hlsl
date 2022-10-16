struct VertexInput
{
    float3 position : POSITION;
    float2 texture_coords : TEXTURE_COORDS;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float2 texture_coords : TEXTURE_COORDS;
};

struct CameraBuffer
{
    row_major matrix view_projection_matrix;
};

[[vk::push_constant]]
ConstantBuffer<CameraBuffer> camera_buffer : register(b0);

VertexOutput vs_main(VertexInput input)
{
    VertexOutput output;
    output.position = mul(float4(input.position, 1.0f), camera_buffer.view_projection_matrix);
    output.texture_coords = input.texture_coords;    

    return output;
}

Texture2D<float4> diffuse_texture: register(t0);
SamplerState anisotropic_sampler: register(s0);

float4 ps_main(VertexOutput input) : SV_Target 
{
    float3 color = diffuse_texture.Sample(anisotropic_sampler, input.texture_coords).xyz;

    return float4(color, 1.0f);
}