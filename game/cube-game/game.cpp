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
            .intensity = 5.0f,
        });

        scene::SceneManager::instance().add_scene(std::move(default_scene));

        // Manually modify material of player a bit.
        // Good indication to add serialization of game objects.
        auto &current_scene = scene::SceneManager::instance().get_current_scene();

        auto &player = current_scene.get_game_object("player");

        player.m_materials[0].material_data.metallic_roughness_factor.x = 0.0f;
        player.m_materials[0].material_data.metallic_roughness_factor.y = 0.0f;
    }

    virtual void update(const float delta_time) override
    {
        static bool lock_camera_to_player = false;

        editor::Editor::instance().add_render_callback([&]() {
            if (ImGui::Begin("Game Settings"))
            {
                ImGui::Checkbox("Lock Cam to Player", &lock_camera_to_player);
                ImGui::End();
            }
        });

        // Ensure the singular point light in scene is always behind the player.
        auto &current_scene = scene::SceneManager::instance().get_current_scene();
        auto &player = current_scene.get_game_object("player");

        auto &player_position = current_scene.get_game_object("player").get_transform_component().translation;

        current_scene.get_lights().get_light_buffer().lights[1].world_space_position_or_direction = {
            player_position.x,
            player_position.y,
            player_position.z - 2.0f,
        };

        // Lock camera to player.
        if (lock_camera_to_player)
        {
            current_scene.get_camera().m_camera_position = {
                player_position.x,
                player_position.y + 2.0f,
                player_position.z - 10.0f,
                1.0f,
            };
        }

        // Custom collision logic.
        // Note that since this game only uses cubes, this will work.
        // Note : The origin is at the center of the cube / cuboid volume.

        // Get the AABB coords for the player (static per frame).
        const auto player_x_min = player_position.x - player.get_transform_component().scale.x;
        const auto player_x_max = player_position.x + player.get_transform_component().scale.x;
        
        const auto player_y_min = player_position.y - player.get_transform_component().scale.y;
        const auto player_y_max = player_position.y + player.get_transform_component().scale.y;

        const auto player_z_min = player_position.z - player.get_transform_component().scale.z;
        const auto player_z_max = player_position.z + player.get_transform_component().scale.z;

        auto player_collisions = 0u;

        for (const auto& [name, game_object] : current_scene.get_game_objects())
        {
            if (game_object.m_game_object_name != player.m_game_object_name)
            {
                const auto &object_translation = game_object.m_transform_component.translation;
                const auto &object_scale = game_object.m_transform_component.scale;

                const auto object_x_min = object_translation.x - object_scale.x;
                const auto object_x_max = object_translation.x + object_scale.x;

                const auto object_y_min = object_translation.y - object_scale.y;
                const auto object_y_max = object_translation.y + object_scale.y;

                const auto object_z_min = object_translation.z - object_scale.z;
                const auto object_z_max = object_translation.z + object_scale.z;

                // Reference : https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection.
                const auto x_axis_collision = object_x_max >= player_x_min && object_x_min <= player_x_max;
                const auto y_axis_collision = object_y_max >= player_y_min && object_y_min <= player_y_max;
                const auto z_axis_collision = object_z_max >= player_z_min && object_z_min <= player_z_max;

                if ((x_axis_collision + y_axis_collision + z_axis_collision) == 3)
                {
                    core::Log::instance().warn(std::format("Collision occured between : {} and {}",
                                                           player.m_game_object_name, game_object.m_game_object_name));

                    player_collisions++;
                }
            }
        }

        if (!player_collisions)
        {
            player_position.y -= 0.001f * delta_time * 9.8;
        }

        const auto projection_matrix = math::XMMatrixPerspectiveFovLH(math::XMConvertToRadians(60.0f),
                                                                      m_window->get_aspect_ratio(), 0.1f, 1000.0f);

        current_scene.update(projection_matrix, delta_time, m_frame_count, m_input);

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