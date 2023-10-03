// clang-format off

#include "interop/render_resources.hlsli"
#include "interop/constant_buffers.hlsli"

#include "utils.hlsli"

#include "atmosphere/preetham_sky_model.hlsli"

ConstantBuffer<interop::AtmosphereRenderResources> render_resources : register(b0);

[numthreads(8, 8, 1)]
void cs_main(uint3 dispatch_thread_id : SV_DispatchThreadID)
{
    ConstantBuffer<interop::LightBuffer> light_buffer = ResourceDescriptorHeap[render_resources.light_buffer_cbv_index];
    ConstantBuffer<interop::AtmosphereRenderPassBuffer> atmosphere_buffer = ResourceDescriptorHeap[render_resources.atmosphere_buffer_cbv_index];
 
    RWTexture2DArray<float4> output_texture = ResourceDescriptorHeap[render_resources.output_texture_uav_index];
    float2 uv = (dispatch_thread_id.xy + 0.5f) / float2(atmosphere_buffer.output_texture_dimension);
  
    const float3 direction = get_sampling_vector_cubemap(uv, dispatch_thread_id.z);

    output_texture[dispatch_thread_id] = float4(preetham_sky_luminance_and_chromaticity(atmosphere_buffer,  
                                                normalize(direction), normalize(light_buffer.lights[interop::SUN_LIGHT_INDEX].world_space_position_or_direction), 
                                                light_buffer.lights[interop::SUN_LIGHT_INDEX].intensity), 1.0f);
}
