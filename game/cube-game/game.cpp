#include "serenity-engine/serenity-engine.hpp"

using namespace serenity;

class CubeGame final : public core::Application
{
  public:
    explicit CubeGame(const core::ApplicationConfig &application_config) : core::Application(application_config)
    {
        // Create default scene.
        // The init_default_level.lua script will contain information of all the 'blocks' of the level, and also include
        // the player.
        const auto scene_init_script_index = scripting::ScriptManager::instance().create_script(scripting::Script{
            .script_name = "init_default_level",
            .script_path = wstring_to_string(
                core::FileSystem::instance().get_absolute_path(L"game/cube-game/scripts/init_default_level.lua")),
        });

        auto default_scene = scene::Scene("Default Scene", scene_init_script_index);

        // Add a point light that is right behind the player cube, but is invisible (i.e scale 0).
        default_scene.add_light(interop::Light{
            .light_type = interop::LightType::Point,
            .color = math::XMFLOAT3A(1.0f, 1.0f, 1.0f),
            .intensity = 1.0f,
        });

        scene::SceneManager::instance().add_scene(std::move(default_scene));

        // Manually modify material of player a bit.
        // Good indication to add serialization of game objects.
        auto &current_scene = scene::SceneManager::instance().get_current_scene();

        current_scene.get_player_object().m_materials[0].material_data.metallic_roughness_factor.x = 0.0f;
        current_scene.get_player_object().m_materials[0].material_data.metallic_roughness_factor.y = 0.0f;
    }

    virtual void update(const float delta_time) override
    {
        // Ensure the singular point light in scene is always behind the player.
        auto &current_scene = scene::SceneManager::instance().get_current_scene();

        const auto &player_position = current_scene.get_player_object().get_transform_component().translation;

        current_scene.get_lights().get_light_buffer().lights[1].world_space_position_or_direction = {
            player_position.x - 5.0f, player_position.y, player_position.z};

        const auto projection_matrix = math::XMMatrixPerspectiveFovLH(math::XMConvertToRadians(60.0f),
                                                                      m_window->get_aspect_ratio(), 0.1f, 1000.0f);

        current_scene.update(projection_matrix, delta_time, m_frame_count,
                                                                   m_input);

        renderer::Renderer::instance().update_renderpasses(m_frame_count);
    }
};

std::unique_ptr<serenity::core::Application> serenity::core::create_application()
{
    return std::make_unique<CubeGame>(serenity::core::ApplicationConfig{
        .log_to_console = true,
        .log_to_file = true,
        .dimensions =
            Float2{
                .x = 100.0f,
                .y = 100.0f,
            },
    });
}