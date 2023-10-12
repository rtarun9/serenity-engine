#include "serenity-engine/scene/scene.hpp"
#include "serenity-engine/asset/model_loader.hpp"

#include "serenity-engine/renderer/renderer.hpp"

namespace serenity::scene
{
    Scene::Scene(const std::string_view scene_name) : m_scene_name(scene_name)
    {
        // Create the scene buffer.
        m_scene_buffer_index = renderer::Renderer::instance().create_buffer<interop::SceneBuffer>(
            renderer::rhi::BufferCreationDesc{.usage = renderer::rhi::BufferUsage::ConstantBuffer,
                                              .name = string_to_wstring(scene_name) + L" Scene Buffer"});

        core::Log::instance().info(std::format("Created scene {}", scene_name));
    }

    void Scene::update(const math::XMMATRIX projection_matrix, const float delta_time, const uint32_t frame_count,
                       const core::Input &input)
    {
        m_camera.update(delta_time, input);

        m_lights.update(m_scene_buffer.view_matrix);

        // Update scene buffer.
        m_scene_buffer.view_projection_matrix = m_camera.get_view_matrix() * projection_matrix;
        m_scene_buffer.inverse_projection_matrix = math::XMMatrixInverse(nullptr, projection_matrix);
        m_scene_buffer.inverse_view_projection_matrix =
            math::XMMatrixInverse(nullptr, m_scene_buffer.view_projection_matrix);
        m_scene_buffer.view_matrix = m_camera.get_view_matrix();
        m_scene_buffer.inverse_view_matrix = math::XMMatrixInverse(nullptr, m_camera.get_view_matrix());

        m_scene_buffer.camera_position =
            math::XMFLOAT3{m_camera.m_camera_position.x, m_camera.m_camera_position.y, m_camera.m_camera_position.z};

        renderer::Renderer::instance()
            .get_buffer_at_index(m_scene_buffer_index)
            .update(reinterpret_cast<const std::byte *>(&m_scene_buffer), sizeof(interop::SceneBuffer));

        for (auto &game_object : m_game_objects)
        {
            game_object.m_transform_component.update(delta_time, frame_count);
        }
    }

    void Scene::add_light(const interop::Light &light)
    {
        m_lights.add_light(light);
    }
} // namespace serenity::scene