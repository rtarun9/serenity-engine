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

ConstantBuffer<PhongShadingRenderResources> render_resources : register(b0);

VsOutput vs_main(uint vertex_id : SV_VertexID)  
{
    StructuredBuffer<float3> position_buffer = ResourceDescriptorHeap[render_resources.position_buffer_srv_index];
    StructuredBuffer<float2> texture_coord_buffer = ResourceDescriptorHeap[render_resources.texture_coord_buffer_srv_index];
    StructuredBuffer<float3> normal_buffer = ResourceDescriptorHeap[render_resources.normal_buffer_srv_index];

    ConstantBuffer<TransformBuffer> transform_buffer = ResourceDescriptorHeap[render_resources.transform_buffer_cbv_index];
    ConstantBuffer<SceneBuffer> scene_buffer= ResourceDescriptorHeap[render_resources.scene_buffer_cbv_index];

    VsOutput output;
    output.position = mul(float4(position_buffer[vertex_id], 1.0f), mul(transform_buffer.model_matrix, scene_buffer.view_projection_matrix));
    output.texture_coord = texture_coord_buffer[vertex_id];
    output.pixel_position = mul(float4(position_buffer[vertex_id], 1.0f), transform_buffer.model_matrix).xyz;
    output.normal = mul(normal_buffer[vertex_id], (float3x3)(transform_buffer.transposed_inverse_model_matrix));
    output.camera_position = scene_buffer.camera_position;

    return output;
}

SamplerState anisotropic_sampler : register(s0);
SamplerState linear_wrap_sampler : register(s1);

float4 ps_main(VsOutput input) : SV_Target0
{
    ConstantBuffer<MaterialBuffer> material_buffer = ResourceDescriptorHeap[render_resources.material_buffer_cbv_index];
    float4 color = material_buffer.base_color;

    if (material_buffer.albedo_texture_srv_index != INVALID_INDEX_U32)
    {
        Texture2D<float4> albedo_texture = ResourceDescriptorHeap[material_buffer.albedo_texture_srv_index];
        //color = albedo_texture.Sample(anisotropic_sampler, input.texture_coord);
    }

    // Perform shading.
    TextureCube<float4> atmosphere_texture = ResourceDescriptorHeap[render_resources.atmosphere_texture_srv_index];

    ConstantBuffer<LightBuffer> light_buffer = ResourceDescriptorHeap[render_resources.light_buffer_cbv_index];
    
    float4 shading_result = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    for (uint i = 0; i < light_buffer.light_count; i++)
    {
        Light light = light_buffer.lights[i];

        // Ambient light: Lights bounces all over the scene, so no part of the object is truly pitch black.
        const float ambient_color_factor = 0.025f;
        float3 ambient_light = ambient_color_factor * light.color;

        // Diffuse component of the phong shading model represents the directional impact of the light.
        const float3 pixel_to_light_direction = normalize(light.world_space_position_or_direction - input.pixel_position);
        const float3 normal = normalize(input.normal);

        float3 diffuse_light = max(dot(pixel_to_light_direction, normal), 0.0f) * light.color;

        // Calculate specular light.
        // There exist a perfect reflection direction. When the view direction equals / is close to this perfect direction
        // the object tends to have a high degree of reflectivity.
        const float3 pixel_to_camera_direction = normalize(input.camera_position - input.pixel_position);
        float3 perfect_reflection_direction = reflect(-pixel_to_light_direction, normal);
        float specular_strength = pow(max(dot(pixel_to_camera_direction, perfect_reflection_direction), 0.0f), 64) * 0.51f;

        float3 specular_light = light.color * specular_strength;

        if (light.light_type == LightType::Directional)
        {
            const float sun_light_ambient_color_factor = 0.0125f;
            ambient_light = atmosphere_texture.Sample(linear_wrap_sampler, normalize(input.pixel_position)).xyz * sun_light_ambient_color_factor;

            const float sun_light_diffuse_color_factor = 0.00025f;
            diffuse_light = max(dot(light.world_space_position_or_direction, normal), 0.0f) * light.color * sun_light_diffuse_color_factor;

            float3 perfect_reflection_direction = reflect(-light.world_space_position_or_direction, normal);
            float specular_strength = pow(max(dot(pixel_to_camera_direction, perfect_reflection_direction), 0.0f), 128) * 0.01f;

            specular_light = light.color * specular_strength;
        }

        shading_result += float4((specular_light + diffuse_light + ambient_light) * color.xyz, 1.0f);
    }

    return shading_result;
}