#include "serenity-engine/scene/scene.hpp"
#include "serenity-engine/asset/model_loader.hpp"
#include "serenity-engine/core/file_system.hpp"
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

    Scene::Scene(const std::string_view scene_name, const uint32_t scene_init_script_index) : Scene(scene_name)
    {
        m_scene_init_script_index = scene_init_script_index;

        scripting::ScriptManager::instance().execute_script(scene_init_script_index);

        sol::table game_objects = scripting::ScriptManager::instance().get_state()["game_objects"];

        for (auto &key_value_pair : game_objects)
        {
            const auto key = key_value_pair.first.as<std::string>();
            const auto value = key_value_pair.second.as<sol::table>();

            const std::string model_path = value["model_path"];

            const math::XMFLOAT3 scale = {value["scale"]["x"], value["scale"]["y"], value["scale"]["z"]};
            const math::XMFLOAT3 rotation = {value["rotation"]["x"], value["rotation"]["y"], value["rotation"]["z"]};
            const math::XMFLOAT3 translation = {value["translation"]["x"], value["translation"]["y"],
                                                value["translation"]["z"]};

            auto new_game_object = GameObject(key, model_path);
            new_game_object.get_transform_component().scale = scale;
            new_game_object.get_transform_component().rotation = rotation;
            new_game_object.get_transform_component().translation = translation;

            const sol::table script = value["script"];
            if (!script.empty())
            {
                const std::string path = script["path"];
                new_game_object.m_script_index = scripting::ScriptManager::instance().create_script(scripting::Script{
                    .script_name = script["name"],
                    .script_path = core::FileSystem::instance().get_absolute_path(path),
                });
            }

            if (key == "player")
            {
                m_player = &new_game_object;
            }

            add_game_object(std::move(new_game_object));
        }

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
            game_object.update(delta_time, frame_count);
        }
    }

    void Scene::reload()
    {
        if (!m_scene_init_script_index.has_value())
        {
            core::Log::instance().warn(
                std::format("Cannot reload scene with name {} that does not have a scene init script", m_scene_name));
            return;
        }

        m_game_objects.clear();

        scripting::ScriptManager::instance().execute_script(m_scene_init_script_index.value());

        sol::table game_objects = scripting::ScriptManager::instance().get_state()["game_objects"];

        for (auto &key_value_pair : game_objects)
        {
            const auto key = key_value_pair.first.as<std::string>();
            const auto value = key_value_pair.second.as<sol::table>();

            const std::string model_path = value["model_path"];

            const math::XMFLOAT3 scale = {value["scale"]["x"], value["scale"]["y"], value["scale"]["z"]};
            const math::XMFLOAT3 rotation = {value["rotation"]["x"], value["rotation"]["y"], value["rotation"]["z"]};
            const math::XMFLOAT3 translation = {value["translation"]["x"], value["translation"]["y"],
                                                value["translation"]["z"]};

            auto new_game_object = GameObject(key, model_path);
            new_game_object.get_transform_component().scale = scale;
            new_game_object.get_transform_component().rotation = rotation;
            new_game_object.get_transform_component().translation = translation;

            const sol::table script = value["script"];
            if (!script.empty())
            {
                const std::string path = script["path"];
                new_game_object.m_script_index = scripting::ScriptManager::instance().create_script(scripting::Script{
                    .script_name = script["name"],
                    .script_path = core::FileSystem::instance().get_absolute_path(path),
                });
            }

            if (key == "player")
            {
                m_player = &new_game_object;
            }

            add_game_object(std::move(new_game_object));
        }

        core::Log::instance().info(std::format("Created scene {}", m_scene_name));
    }

    void Scene::add_light(const interop::Light &light)
    {
        m_lights.add_light(light);
    }
} // namespace serenity::scene