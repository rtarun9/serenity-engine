#include "serenity-engine/serenity-engine.hpp"

using namespace serenity;

class CubeGame final : public core::Application
{
  private:
    bool m_play_game{false};
    bool m_first_game_frame{true};
    bool m_lock_camera_to_player{false};
    float m_player_speed{0.051f};
    uint32_t m_fails{};

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

        // Add scene 2 (with the obstacles in reverse position).
        const auto scene_level_2_init_script_index = scripting::ScriptManager::instance().create_script(scripting::Script{
            .script_name = "init_level_2",
            .script_path = wstring_to_string(
                core::FileSystem::instance().get_absolute_path(L"game/cube-game/scripts/init_level_2.lua")),
        });

        auto level_2 = scene::Scene("Level 2", scene_level_2_init_script_index);

        // Add a point light that is right behind the player cube, but is invisible (i.e scale 0).
        level_2.add_light(interop::Light{
            .light_type = interop::LightType::Point,
            .color = math::XMFLOAT3A(1.0f, 1.0f, 1.0f),
            .intensity = 5.0f,
        });

        level_2.get_camera().m_movement_speed = 0.032f;

        scene::SceneManager::instance().add_scene(std::move(level_2));

        // Manually modify material of player a bit.
        // Good indication to add serialization of game objects.
        auto &current_scene = scene::SceneManager::instance().get_current_scene();
        current_scene.get_camera().m_movement_speed = 0.032f;

        auto &player = current_scene.get_game_object("player");

        player.m_materials[0].material_data.metallic_roughness_factor.x = 0.0f;
        player.m_materials[0].material_data.metallic_roughness_factor.y = 0.0f;
    }

    virtual void update(const float delta_time) override
    {
        // References to frequently accessed things.
        auto &current_scene = scene::SceneManager::instance().get_current_scene();
        auto &player = current_scene.get_game_object("player");
        auto &player_position = current_scene.get_game_object("player").get_transform_component().translation;

        // Game settings UI.
        editor::Editor::instance().add_render_callback(
            [&]() {
                if (ImGui::Begin("Game Settings"))
                {
                    ImGui::Text(("Current Level : "s + current_scene.get_scene_name()).c_str());

                    ImGui::Text("Fails : %d", m_fails);

                    ImGui::Checkbox("Lock Cam to Player", &m_lock_camera_to_player);

                    ImGui::SliderFloat("Player Speed", &m_player_speed, 0.0f, 10.0f);

                    if (ImGui::Button(!m_play_game ? "Play Game" : "Stop Game"))
                    {
                        m_first_game_frame = true;
                        m_play_game = !m_play_game;
                    }

                    if (ImGui::Button("Reload"))
                    {
                        m_play_game = false;
                        m_fails = 0;
                        current_scene.reload();
                    }

                    if (ImGui::Button("Change Level"))
                    {
                        if (current_scene.get_scene_name() == "Default Scene")
                        {
                            scene::SceneManager::instance().set_current_scene("Level 2");
                        }
                        else
                        {
                            scene::SceneManager::instance().set_current_scene("Default Scene");
                        }

                        current_scene.get_camera().m_movement_speed = 0.032f;
                    }

                    ImGui::End();
                }
            },
            editor::UIType::Game);

        // Ensure the singular point light in scene is always behind the player.
        current_scene.get_lights().get_light_buffer().lights[1].world_space_position_or_direction = {
            player_position.x,
            player_position.y,
            player_position.z - 2.0f,
        };

        // Lock camera to player.
        if (m_lock_camera_to_player)
        {
            current_scene.get_camera().m_camera_position = {
                current_scene.get_camera().m_camera_position.x,
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

        for (const auto &[name, game_object] : current_scene.get_game_objects())
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
                    player_collisions++;
                }
            }
        }

        // If the game is being played and its the first frame, lock the camera position to the player.
        if (m_play_game)
        {
            if (m_first_game_frame)
            {
                current_scene.get_camera().m_camera_position = {
                    player_position.x,
                    player_position.y + 2.0f,
                    player_position.z - 10.0f,
                    1.0f,
                };

                m_first_game_frame = false;
            }

            m_lock_camera_to_player = true;

            // If the player hasn't collided anything (including the floor, then apply gravity - like effects)
            if (!player_collisions)
            {
                player_position.y -= 0.001f * delta_time * 9.8;
            }

            // Player collision will be 1 for the floor, so if its more than 1 a obstacle was hit.
            if (player_collisions > 1)
            {
                ++m_fails;
                current_scene.reload();
            }

            // Update player position (horizontal) based on camera position.
            player_position.x = current_scene.get_camera().m_camera_position.x;

            // Move the player forward (since thats whats the game is about).
            player_position.z += m_player_speed * delta_time;

            // The hardcoded value 350 is the end of the floor region.
            // If the player crosses that, they have won the game.
            if (player_position.z >= 350.0f)
            {
                // If the scene is Default Scene, then move on to the next level.
                if (current_scene.get_scene_name() == "Default Scene")
                {
                    scene::SceneManager::instance().set_current_scene("Level 2");
                }
                else
                {
                    // Otherwise, the game is over.
                    m_lock_camera_to_player = false;

                    editor::Editor::instance().add_render_callback(
                        [&]() {
                            ImGui::Begin("You Win!");

                            ImGui::Text("Congratulations, you win!");

                            if (ImGui::Button("Press to restart"))
                            {
                                current_scene.reload();
                            }

                            ImGui::End();
                        },
                        editor::UIType::Game);
                }
            }
            else
            {
                // If the player hasn't reached the end and falls down, then restart the level as they failed.
                // If player goes below the floor, reload scene.
                if (player_position.y <= -5.0f)
                {
                    m_fails++;
                    current_scene.reload();
                }
            }
        }

        // Non-game specific updates.
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