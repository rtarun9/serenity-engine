// clang-format off

#include "interop/render_resources.hlsli"
#include "interop/constant_buffers.hlsli"

#include "shading/brdf.hlsli"
#include "utils.hlsli"

// All calculations are done in world space.
struct VsOutput
{
    float4 position : SV_Position;
    float3 pixel_position : WORLD_SPACE_POSITION;
    float2 texture_coord : TEXTURE_COORD;
    float3 normal: NORMAL;
    float3 camera_position: CAMERA_POSITION;
};

ConstantBuffer<interop::PBRShadingRenderResources> render_resources : register(b0);

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

struct PBRShadingParams
{
    float3 normal;
    float3 pixel_to_light_direction;
    float3 pixel_to_camera_direction;
    float3 albedo;
    float roughness_factor;
    float metallic_factor;
};

float3 compute_pbr_lighting(PBRShadingParams params, const float3 light_color)
{
    const float3 brdf = 
        cook_torrence_specular_BRDF(params.normal, params.pixel_to_camera_direction, params.pixel_to_light_direction, params.albedo, params.roughness_factor, params.metallic_factor) +
        lambertian_diffuse_BRDF(params.normal, params.pixel_to_camera_direction, params.pixel_to_light_direction, params.albedo, params.roughness_factor, params.metallic_factor);


    return brdf * light_color * saturate(dot(params.pixel_to_light_direction, params.normal));

}

float4 ps_main(VsOutput input) : SV_Target0
{
    ConstantBuffer<interop::MaterialBuffer> material_buffer = ResourceDescriptorHeap[render_resources.material_buffer_cbv_index];
    float4 color = material_buffer.base_color;
    
    if (material_buffer.albedo_texture_srv_index != interop::INVALID_INDEX_U32)
    {
        Texture2D<float4> albedo_texture = ResourceDescriptorHeap[material_buffer.albedo_texture_srv_index];
        color *= albedo_texture.Sample(anisotropic_sampler, input.texture_coord);
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
            PBRShadingParams params;
            params.normal = normal;
            params.pixel_to_light_direction = normalize(light.world_space_position_or_direction - input.pixel_position);
            params.pixel_to_camera_direction = normalize(input.camera_position - input.pixel_position);
            params.albedo = color.xyz;
            params.metallic_factor = material_buffer.metallic_roughness_factor.x;
            params.roughness_factor = material_buffer.metallic_roughness_factor.y;

            const float attenuation_factor = 1.0f / length(light.world_space_position_or_direction - input.pixel_position);
            
            shading_result += compute_pbr_lighting(params, light.color * light.intensity) * attenuation_factor;
        }
        else if (light.light_type == interop::LightType::Directional)
        {
            PBRShadingParams params;
            params.normal = normal;
            params.pixel_to_light_direction = normalize(light.world_space_position_or_direction);
            params.pixel_to_camera_direction = normalize(input.camera_position - input.pixel_position);
            params.albedo = color.xyz;
            params.metallic_factor = material_buffer.metallic_roughness_factor.x;
            params.roughness_factor = material_buffer.metallic_roughness_factor.y;

            shading_result += compute_pbr_lighting(params, atmosphere_texture.Sample(linear_wrap_sampler, input.pixel_position).xyz * light.intensity);   
        }
    }

    return float4(shading_result, 1.0f);
}