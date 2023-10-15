#include "serenity-engine/renderer/renderpass/atmosphere_renderpass.hpp"

#include "serenity-engine/renderer/renderer.hpp"

#include "shaders/interop/render_resources.hlsli"

namespace serenity::renderer::renderpass
{
    AtmosphereRenderpass::AtmosphereRenderpass()
    {
        core::Log::instance().info("Created atmosphere render pass");

        // Create buffers.
        m_atmosphere_buffer_index =
            Renderer::instance().create_buffer<interop::AtmosphereRenderPassBuffer>(rhi::BufferCreationDesc{
                .usage = rhi::BufferUsage::ConstantBuffer,
                .name = L"Atmosphere Render Pass buffer",
            });

        m_atmosphere_buffer_data.turbidity = 2.0f;
        m_atmosphere_buffer_data.output_texture_dimension = {static_cast<float>(ATMOSPHERE_TEXTURE_DIMENSION),
                                                             static_cast<float>(ATMOSPHERE_TEXTURE_DIMENSION)};
        m_atmosphere_buffer_data.magnitude_multiplier = 0.034f;

        // Create texture.
        m_atmosphere_texture_index = Renderer::instance().create_texture(rhi::TextureCreationDesc{
            .usage = rhi::TextureUsage::UAVTexture,
            .format = DXGI_FORMAT_R16G16B16A16_FLOAT,
            .bytes_per_pixel = 8u,
            .array_size = 6u,
            .dimension = {ATMOSPHERE_TEXTURE_DIMENSION, ATMOSPHERE_TEXTURE_DIMENSION},
            .name = L"Atmosphere Texture",
        });

        // Create pipeline objects.
        m_preetham_sky_generation_pipeline_index = Renderer::instance().create_pipeline(rhi::PipelineCreationDesc{
            .pipeline_variant = rhi::PipelineVariant::Compute,
            .compute_shader_creation_desc =
                ShaderCreationDesc{
                    .shader_type = ShaderTypes::Compute,
                    .shader_path = L"shaders/atmosphere/atmosphere.hlsl",
                    .shader_entry_point = L"cs_main",
                },
            .name = L"Atmosphere Pipeline",
        });
    }

    AtmosphereRenderpass::~AtmosphereRenderpass()
    {
        core::Log::instance().info("Destroyed atmosphere render pass");
    }

    void AtmosphereRenderpass::update(const math::XMFLOAT3 sun_direction)
    {
        compute_perez_parameters();

        compute_zenith_luminance(sun_direction);

        Renderer::instance()
            .get_buffer_at_index(m_atmosphere_buffer_index)
            .update(reinterpret_cast<const std::byte *>(&m_atmosphere_buffer_data), sizeof(interop::AtmosphereRenderPassBuffer));
    }

    void AtmosphereRenderpass::compute(rhi::CommandList &command_list, const uint32_t scene_buffer_cbv_index,
                                       const uint32_t light_buffer_cbv_index) const
    {
        // Generate atmosphere texture cube.
        command_list.add_resource_barrier(
            Renderer::instance().get_texture_at_index(m_atmosphere_texture_index).resource.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        command_list.execute_barriers();

        command_list.set_bindless_compute_root_signature();
        command_list.set_pipeline_state(
            renderer::Renderer::instance().get_pipeline_at_index(m_preetham_sky_generation_pipeline_index));

        const auto atmosphere_render_resources = interop::AtmosphereRenderResources{
            .light_buffer_cbv_index = light_buffer_cbv_index,
            .atmosphere_buffer_cbv_index =
                Renderer::instance().get_buffer_at_index(m_atmosphere_buffer_index).cbv_index,
            .output_texture_uav_index = Renderer::instance().get_texture_at_index(m_atmosphere_texture_index).uav_index,
        };

        command_list.set_compute_32_bit_root_constants(
            reinterpret_cast<const std::byte *>(&atmosphere_render_resources));

        command_list.dispatch(Uint3{
            .x = std::max(static_cast<uint32_t>(std::ceilf(ATMOSPHERE_TEXTURE_DIMENSION / 8u)), 0u),
            .y = std::max(static_cast<uint32_t>(std::ceilf(ATMOSPHERE_TEXTURE_DIMENSION / 8u)), 0u),
            .z = 6u,
        });

        command_list.add_resource_barrier(
            Renderer::instance().get_texture_at_index(m_atmosphere_texture_index).resource.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        command_list.execute_barriers();
    }

    void AtmosphereRenderpass::compute_perez_parameters()
    {
        // Formulas given in section A.2 of the A.J. Preetham paper.
        const auto turbidity = m_atmosphere_buffer_data.turbidity;

        // Calculation of Y, x and y of perez parameters.
        m_atmosphere_buffer_data.perez_parameters.A = math::XMFLOAT3(
            0.1787f * turbidity - 1.4630f, -0.0193f * turbidity - 0.2592f, -0.0167f * turbidity - 0.2608f);

        m_atmosphere_buffer_data.perez_parameters.B = math::XMFLOAT3(
            -0.3554f * turbidity + 0.4275f, -0.0665f * turbidity + 0.0008f, -0.0950f * turbidity + 0.0092f);

        m_atmosphere_buffer_data.perez_parameters.C = math::XMFLOAT3(
            -0.0227f * turbidity + 5.3251f, -0.0004f * turbidity + 0.2125f, -0.0079f * turbidity + 0.2102f);

        m_atmosphere_buffer_data.perez_parameters.D = math::XMFLOAT3(
            0.1206f * turbidity - 2.5771f, -0.0641f * turbidity - 0.8989f, -0.0441f * turbidity - 1.6537f);

        m_atmosphere_buffer_data.perez_parameters.E = math::XMFLOAT3(
            -0.0670f * turbidity + 0.3703f, -0.0033f * turbidity + 0.0452f, -0.0109f * turbidity + 0.0529);
    }

    void AtmosphereRenderpass::compute_zenith_luminance(const math::XMFLOAT3 sun_direction)
    {
        // Theta_s is angle between sun and zenith, but while computed sun_direction, angle is between horizontal
        // axis.
        const auto theta_s = acosf(std::clamp(sun_direction.y, 0.0f, 1.0f));

        const auto turbidity = m_atmosphere_buffer_data.turbidity;

        const auto chi = (4.0f / 9.0f - turbidity / 120.0f) * (math::XM_PI - 2.0f * theta_s);

        m_atmosphere_buffer_data.zenith_luminance_chromaticity.x =
            (4.0453f * turbidity - 4.9710f) * tanf(chi) - 0.2155f * turbidity + 2.4192f;

        const auto theta_s_vector = math::XMFLOAT4(pow(theta_s, 3), pow(theta_s, 2), pow(theta_s, 1), 1);

        const auto turbidity_vector = math::XMFLOAT3(pow(turbidity, 2), turbidity, 1);

        m_atmosphere_buffer_data.zenith_luminance_chromaticity.y =
            (turbidity_vector.x * (0.0017f * theta_s_vector.x - 0.00375f * theta_s_vector.y +
                                   0.0021f * theta_s_vector.z + 0.0f * theta_s_vector.w)) +
            (turbidity_vector.y * (-0.0290f * theta_s_vector.x + 0.0638f * theta_s_vector.y -
                                   0.0320f * theta_s_vector.z + 0.0039f * theta_s_vector.w)) +
            (turbidity_vector.z * (0.1169f * theta_s_vector.x - 0.2120f * theta_s_vector.y +
                                   0.0605f * theta_s_vector.z + 0.2589f * theta_s_vector.w));

        m_atmosphere_buffer_data.zenith_luminance_chromaticity.z =
            (turbidity_vector.x * (0.0028f * theta_s_vector.x - 0.0061f * theta_s_vector.y +
                                   0.0032f * theta_s_vector.z + 0.0f * theta_s_vector.w)) +
            (turbidity_vector.y * (-0.0421 * theta_s_vector.x + 0.0897f * theta_s_vector.y -
                                   0.0415f * theta_s_vector.z + 0.0052f * theta_s_vector.w)) +
            (turbidity_vector.z * (0.1535f * theta_s_vector.x - 0.2676f * theta_s_vector.y +
                                   0.0667f * theta_s_vector.z + 0.2669f * theta_s_vector.w));
    }
} // namespace serenity::renderer::renderpass