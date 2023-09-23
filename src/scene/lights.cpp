#include "serenity-engine/scene/lights.hpp"

#include "serenity-engine/renderer/renderer.hpp"

namespace serenity::scene
{
    Lights::Lights()
    {
        // Create light buffer.
        m_light_buffer_index =
            renderer::Renderer::instance().create_buffer<LightBuffer>(renderer::rhi::BufferCreationDesc{
                .usage = renderer::rhi::BufferUsage::ConstantBuffer,
                .name = L"Light Buffer Index",
            });

        m_light_buffer.sun_angle = -90.0f;

        // Add a directional light at the start.
        add_light(Light{
            .light_type = LightType::Directional,
            .view_space_position_or_direction = {0.0f, sinf(math::XMConvertToRadians(m_light_buffer.sun_angle)),
                                                 cosf(math::XMConvertToRadians(m_light_buffer.sun_angle))},
            .color = math::XMFLOAT3{1.0f, 1.0f, 1.0f},
            .intensity = 1.0f,
            .size = 0.0f,
        });
    }

    void Lights::add_light(const Light &light)
    {
        if (m_light_buffer.light_count >= MAX_LIGHT_COUNT)
        {
            core::Log::instance().warn("Not adding light the MAX_LIGHT_COUNT is already reached.");
            return;
        }

        // Add light to light buffer.
        m_light_buffer.lights[m_light_buffer.light_count++] = light;
    }

    void Lights::update()
    {
        auto &sun_direction = m_light_buffer.lights[SUN_LIGHT_INDEX].view_space_position_or_direction;

        sun_direction = {0.0f, -1.0f * sinf(math::XMConvertToRadians(m_light_buffer.sun_angle)),
                         -1.0f * cosf(math::XMConvertToRadians(m_light_buffer.sun_angle))};

        const auto magnitude = std::sqrtf(sun_direction.x * sun_direction.x + sun_direction.y * sun_direction.y +
                                          sun_direction.z * sun_direction.z);

        // Normalizing the sun direction.
        m_light_buffer.lights[SUN_LIGHT_INDEX].view_space_position_or_direction = {
            sun_direction.x / magnitude, sun_direction.y / magnitude, sun_direction.z / magnitude};

        renderer::Renderer::instance()
            .get_buffer_at_index(m_light_buffer_index)
            .update(reinterpret_cast<const std::byte *>(&m_light_buffer), sizeof(LightBuffer));
    }
} // namespace serenity::scene