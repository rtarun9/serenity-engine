#include "serenity-engine/scene/lights.hpp"

#include "serenity-engine/renderer/renderer.hpp"

#include "shaders/interop/render_resources.hlsli"

namespace serenity::scene
{
    Lights::Lights()
    {
        // Create light buffer.
        m_light_buffer_index =
            renderer::Renderer::instance().create_buffer<interop::LightBuffer>(renderer::rhi::BufferCreationDesc{
                .usage = renderer::rhi::BufferUsage::ConstantBuffer,
                .name = L"Light Buffer Index",
            });

        m_light_buffer.sun_angle = -90.0f;

        // Add a directional light at the start.
        add_light(interop::Light{
            .light_type = interop::LightType::Directional,
            .world_space_position_or_direction = {0.0f, sinf(math::XMConvertToRadians(m_light_buffer.sun_angle)),
                                                  cosf(math::XMConvertToRadians(m_light_buffer.sun_angle))},
            .color = math::XMFLOAT3{1.0f, 1.0f, 1.0f},
            .intensity = 5.8f,
            .scale = 0.0f,
        });

        // Load cube data (positions and indices) for visualization purposes.
        constexpr auto positions = std::array{
            math::XMFLOAT3(-1.0f, -1.0f, -1.0f), math::XMFLOAT3(-1.0f, +1.0f, -1.0f),
            math::XMFLOAT3(+1.0f, +1.0f, -1.0f), math::XMFLOAT3(+1.0f, -1.0f, -1.0f),
            math::XMFLOAT3(-1.0f, -1.0f, +1.0f), math::XMFLOAT3(-1.0f, +1.0f, +1.0f),
            math::XMFLOAT3(+1.0f, +1.0f, +1.0f), math::XMFLOAT3(+1.0f, -1.0f, +1.0f),
        };

        m_cube_position_buffer_index = renderer::Renderer::instance().create_buffer<math::XMFLOAT3>(
            renderer::rhi::BufferCreationDesc{
                .usage = renderer::rhi::BufferUsage::StructuredBuffer,
                .name = L"Light Cube position buffer",
            },
            positions);

        // Create index array
        constexpr auto indices = std::array<uint16_t, 36>{
            0, 1, 2, 0, 2, 3, 4, 6, 5, 4, 7, 6, 4, 5, 1, 4, 1, 0, 3, 2, 6, 3, 6, 7, 1, 5, 6, 1, 6, 2, 4, 0, 3, 4, 3, 7,
        };

        m_cube_index_buffer_index = renderer::Renderer::instance().create_buffer<uint16_t>(
            renderer::rhi::BufferCreationDesc{
                .usage = renderer::rhi::BufferUsage::StructuredBuffer,
                .name = L"Light Cube index buffer",
            },
            indices);

        // Create light pipeline.
        m_light_pipeline_index = renderer::Renderer::instance().create_pipeline(renderer::rhi::PipelineCreationDesc{
            .pipeline_variant = renderer::rhi::PipelineVariant::Graphics,
            .vertex_shader_creation_desc =
                renderer::ShaderCreationDesc{
                    .shader_type = renderer::ShaderTypes::Vertex,
                    .shader_path = L"shaders/lights.hlsl",
                    .shader_entry_point = L"vs_main",
                },
            .pixel_shader_creation_desc =
                renderer::ShaderCreationDesc{
                    .shader_type = renderer::ShaderTypes::Pixel,
                    .shader_path = L"shaders/lights.hlsl",
                    .shader_entry_point = L"ps_main",
                },
            .rtv_formats = {DXGI_FORMAT_R16G16B16A16_FLOAT},
            .dsv_format = DXGI_FORMAT_D32_FLOAT,
            .name = L"Light Pipeline",
        });
    }

    void Lights::add_light(const interop::Light &light)
    {
        if (m_light_buffer.light_count >= interop::MAX_LIGHT_COUNT)
        {
            core::Log::instance().warn("Not adding light the MAX_LIGHT_COUNT is already reached.");
            return;
        }

        if (light.light_type == interop::LightType::Directional &&
            m_light_buffer.light_count > interop::SUN_LIGHT_INDEX)
        {
            core::Log::instance().warn("Not adding light since directional light already exists (only 1 such light is "
                                       "allowed in the engine currently).");
            return;
        }

        // Add light to light buffer.
        m_light_buffer.lights[m_light_buffer.light_count++] = light;
    }

    void Lights::update(const math::XMMATRIX view_matrix)
    {
        auto &sun_direction = m_light_buffer.lights[interop::SUN_LIGHT_INDEX].world_space_position_or_direction;

        sun_direction = {0.0f, -1.0f * sinf(math::XMConvertToRadians(m_light_buffer.sun_angle)),
                         -1.0f * cosf(math::XMConvertToRadians(m_light_buffer.sun_angle))};

        const auto magnitude = std::sqrtf(sun_direction.x * sun_direction.x + sun_direction.y * sun_direction.y +
                                          sun_direction.z * sun_direction.z);

        // Normalizing the sun direction.
        m_light_buffer.lights[interop::SUN_LIGHT_INDEX].world_space_position_or_direction = {
            sun_direction.x / magnitude, sun_direction.y / magnitude, sun_direction.z / magnitude};

        // For the model matrix, since we use instanced rendering, index 0 in the model matrix array actually
        // corresponds to light at index 1. This is because directional lights do not have a visualization cube to be
        // rendered.
        for (int i = 1; i < m_light_buffer.light_count; i++)
        {
            const auto &light_world_space_position_or_direction =
                m_light_buffer.lights[i].world_space_position_or_direction;

            const auto &light_scale = m_light_buffer.lights[i].scale;

            m_light_buffer.model_matrix[i - 1] = math::XMMatrixScaling(light_scale, light_scale, light_scale) *
                                                 math::XMMatrixTranslation(light_world_space_position_or_direction.x,
                                                                           light_world_space_position_or_direction.y,
                                                                           light_world_space_position_or_direction.z);
        }

        renderer::Renderer::instance()
            .get_buffer_at_index(m_light_buffer_index)
            .update(reinterpret_cast<const std::byte *>(&m_light_buffer), sizeof(interop::LightBuffer));
    }

    void Lights::render(const renderer::rhi::CommandList &command_list, const uint32_t scene_buffer_cbv_index)
    {
        // Set pipeline and root signature state.
        command_list.set_bindless_graphics_root_signature();
        command_list.set_pipeline_state(renderer::Renderer::instance().get_pipeline_at_index(m_light_pipeline_index));

        const auto light_render_resources = interop::LightRenderResources{
            .scene_buffer_cbv_index = scene_buffer_cbv_index,
            .light_buffer_cbv_index =
                renderer::Renderer::instance().get_buffer_at_index(m_light_buffer_index).cbv_index,
            .light_cube_position_buffer_srv_index =
                renderer::Renderer::instance().get_buffer_at_index(m_cube_position_buffer_index).srv_index,
        };

        command_list.set_graphics_32_bit_root_constants(reinterpret_cast<const std::byte *>(&light_render_resources));

        command_list.set_index_buffer(renderer::Renderer::instance().get_buffer_at_index(m_cube_index_buffer_index));
        command_list.draw_indexed_instanced(36, m_light_buffer.light_count - 1u);
    }
} // namespace serenity::scene