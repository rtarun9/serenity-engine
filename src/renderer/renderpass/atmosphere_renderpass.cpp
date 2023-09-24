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
            Renderer::instance().create_buffer<AtmosphereRenderPassBuffer>(rhi::BufferCreationDesc{
                .usage = rhi::BufferUsage::ConstantBuffer,
                .name = L"Atmosphere Render Pass buffer",
            });

        m_atmosphere_buffer_data.turbidity = 4.0f;
        m_atmosphere_buffer_data.magnitude_multiplier = 0.019f;

        // Create pipeline object.
        m_preetham_sky_pipeline_index = Renderer::instance().create_pipeline(rhi::PipelineCreationDesc{
            .pipeline_variant = rhi::PipelineVariant::Graphics,
            .vertex_shader_creation_desc =
                ShaderCreationDesc{
                    .shader_type = ShaderTypes::Vertex,
                    .shader_path = L"shaders/atmosphere/atmosphere.hlsl",
                    .shader_entry_point = L"vs_main",
                },
            .pixel_shader_creation_desc =
                ShaderCreationDesc{
                    .shader_type = ShaderTypes::Pixel,
                    .shader_path = L"shaders/atmosphere/atmosphere.hlsl",
                    .shader_entry_point = L"ps_main",
                },
            .cull_mode = D3D12_CULL_MODE_FRONT,
            .rtv_formats = {DXGI_FORMAT_R16G16B16A16_FLOAT},
            .dsv_format = DXGI_FORMAT_D32_FLOAT,
            .name = L"Preetham Sky Atmosphere Pipeline",
        });

        // Load cubemap data (positions and indices).
        constexpr auto positions = std::array{
            math::XMFLOAT3(-1.0f, -1.0f, -1.0f), math::XMFLOAT3(-1.0f, +1.0f, -1.0f),
            math::XMFLOAT3(+1.0f, +1.0f, -1.0f), math::XMFLOAT3(+1.0f, -1.0f, -1.0f),
            math::XMFLOAT3(-1.0f, -1.0f, +1.0f), math::XMFLOAT3(-1.0f, +1.0f, +1.0f),
            math::XMFLOAT3(+1.0f, +1.0f, +1.0f), math::XMFLOAT3(+1.0f, -1.0f, +1.0f),
        };

        m_cubemap_position_buffer_index = Renderer::instance().create_buffer<math::XMFLOAT3>(
            rhi::BufferCreationDesc{
                .usage = rhi::BufferUsage::StructuredBuffer,
                .name = L"Cubemap position buffer",
            },
            positions);

        // Create index array
        constexpr auto indices = std::array<uint16_t, 36>{
            0, 1, 2, 0, 2, 3, 4, 6, 5, 4, 7, 6, 4, 5, 1, 4, 1, 0, 3, 2, 6, 3, 6, 7, 1, 5, 6, 1, 6, 2, 4, 0, 3, 4, 3, 7,
        };

        m_cubemap_index_buffer_index = Renderer::instance().create_buffer<uint16_t>(
            rhi::BufferCreationDesc{
                .usage = rhi::BufferUsage::StructuredBuffer,
                .name = L"Cubemap index buffer",
            },
            indices);
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
            .update(reinterpret_cast<const std::byte *>(&m_atmosphere_buffer_data), sizeof(AtmosphereRenderPassBuffer));
    }

    void AtmosphereRenderpass::render(rhi::CommandList &command_list, const uint32_t scene_buffer_cbv_index,
                                      const uint32_t light_buffer_cbv_index) const
    {
        // Set pipeline and root signature state.
        command_list.set_bindless_graphics_root_signature();
        command_list.set_pipeline_state(
            renderer::Renderer::instance().get_pipeline_at_index(m_preetham_sky_pipeline_index));

        const auto atmosphere_render_resources = AtmosphereRenderResources{
            .position_buffer_srv_index =
                Renderer::instance().get_buffer_at_index(m_cubemap_position_buffer_index).srv_index,
            .scene_buffer_cbv_index = scene_buffer_cbv_index,
            .light_buffer_cbv_index = light_buffer_cbv_index,
            .atmosphere_buffer_cbv_index =
                Renderer::instance().get_buffer_at_index(m_atmosphere_buffer_index).cbv_index,
        };

        command_list.set_graphics_32_bit_root_constants(
            reinterpret_cast<const std::byte *>(&atmosphere_render_resources));

        command_list.set_index_buffer(Renderer::instance().get_buffer_at_index(m_cubemap_index_buffer_index));
        command_list.draw_indexed_instanced(36, 1u);
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
        // Theta_s is angle between sun and zenith, but while computed sun_direction, angle is between horizontal axis.
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