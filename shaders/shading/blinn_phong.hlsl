// clang-format off

#include "interop/render_resources.hlsli"
#include "interop/constant_buffers.hlsli"

// All calculations are done in world space.
struct VsOutput
{
    float4 position : SV_Position;
    float3 pixel_position : WORLD_SPACE_POSITION;
    float2 texture_coord : TEXTURE_COORD;
    float3 normal: NORMAL;
    float3 camera_position: CAMERA_POSITION;
};

ConstantBuffer<interop::PhongShadingRenderResources> render_resources : register(b0);

VsOutput vs_main(uint vertex_id : SV_VertexID)  
{
    StructuredBuffer<float3> position_buffer = ResourceDescriptorHeap[render_resources.position_buffer_srv_index];
    StructuredBuffer<float2> texture_coord_buffer = ResourceDescriptorHeap[render_resources.texture_coord_buffer_srv_index];
    StructuredBuffer<float3> normal_buffer = ResourceDescriptorHeap[render_resources.normal_buffer_srv_index];

    ConstantBuffer<interop::TransformBuffer> transform_buffer = ResourceDescriptorHeap[render_resources.transform_buffer_cbv_index];
    ConstantBuffer<interop::SceneBuffer> scene_buffer= ResourceDescriptorHeap[render_resources.scene_buffer_cbv_index];

    VsOutput output;

    output.position = mul(float4(position_buffer[vertex_id], 1.0f), mul(transform_buffer.model_matrix, scene_buffer.view_projection_matrix));
    output.pixel_position = mul(float4(position_buffer[vertex_id], 1.0f), transform_buffer.model_matrix).xyz;
    
    output.texture_coord = texture_coord_buffer[vertex_id];
    output.normal = mul(normal_buffer[vertex_id], (float3x3)(transform_buffer.transposed_inverse_model_matrix));
    
    output.camera_position = scene_buffer.camera_position;

    return output;
}

SamplerState anisotropic_sampler : register(s0);
SamplerState linear_wrap_sampler : register(s1);

struct PhongShadingParams
{
    float3 normal;
    float3 pixel_to_light_direction;
    float3 pixel_to_camera_direction;
    float ambient_multipler;
    float diffuse_multiplier;
    float specular_multiplier;
};

float3 compute_blinn_phong_lighting(PhongShadingParams params, const float3 light_color)
{
    // Ambient light: Lights bounces all over the scene, so no part of the object is truly pitch black.
    const float3 ambient_light = params.ambient_multipler * float3(1.0f, 1.0f, 1.0f);

    // Diffuse component of the phong shading model represents the directional impact of the light.
    const float3 diffuse_light = max(dot(params.pixel_to_light_direction, params.normal), 0.0f) * light_color * params.diffuse_multiplier;    

    // Calculate specular light.
    const float3 halfway_vec = normalize(params.pixel_to_camera_direction + params.pixel_to_light_direction);
    const float specular_strength = pow(max(dot(params.normal, halfway_vec), 0.0f), 8) * params.specular_multiplier;

    const float3 specular_light = light_color * specular_strength;

    return (ambient_light + diffuse_light + specular_light);
}

float4 ps_main(VsOutput input) : SV_Target0
{
    ConstantBuffer<interop::MaterialBuffer> material_buffer = ResourceDescriptorHeap[render_resources.material_buffer_cbv_index];
    float4 color = material_buffer.base_color;

    if (material_buffer.albedo_texture_srv_index != interop::INVALID_INDEX_U32)
    {
        Texture2D<float4> albedo_texture = ResourceDescriptorHeap[material_buffer.albedo_texture_srv_index];
        // color = albedo_texture.Sample(anisotropic_sampler, input.texture_coord);
    }

    const float3 normal = normalize(input.normal);

    // Perform shading.
    TextureCube<float4> atmosphere_texture = ResourceDescriptorHeap[render_resources.atmosphere_texture_srv_index];

    ConstantBuffer<interop::LightBuffer> light_buffer = ResourceDescriptorHeap[render_resources.light_buffer_cbv_index];
    
    float3 shading_result = float3(0.0f, 0.0f, 0.0f);
    
    for (uint i = 0; i < light_buffer.light_count; i++)
    {
        interop::Light light = light_buffer.lights[i];

        if (light.light_type == interop::LightType::Point)
        {
            PhongShadingParams params;
            params.normal = normal;
            params.pixel_to_light_direction = normalize(light.world_space_position_or_direction - input.pixel_position);
            params.pixel_to_camera_direction = normalize(input.camera_position - input.pixel_position);
            params.ambient_multipler = 0.1f;
            params.diffuse_multiplier = 1.0f;
            params.specular_multiplier = 0.5f;

            const float attenuation_factor = 1.0f / length(light.world_space_position_or_direction - input.pixel_position);
            
            shading_result += compute_blinn_phong_lighting(params, light.color * light.intensity) * attenuation_factor * color.xyz;
        }
        else if (light.light_type == interop::LightType::Directional)
        {
            PhongShadingParams params;
            params.normal = normal;
            params.pixel_to_light_direction = normalize(light.world_space_position_or_direction);
            params.pixel_to_camera_direction = normalize(input.camera_position - input.pixel_position);
            params.ambient_multipler = 0.025f;
            params.diffuse_multiplier = 0.10f;
            params.specular_multiplier = 0.025f;

            shading_result += compute_blinn_phong_lighting(params, atmosphere_texture.Sample(linear_wrap_sampler, input.pixel_position).xyz) * color.xyz;   
        }
    }

    return float4(shading_result, 1.0f);
}