#include "serenity-engine/serenity-engine.hpp"

using namespace serenity;

class SandBox final : public core::Application
{

  public:
    explicit SandBox(const core::ApplicationConfig &application_config) : core::Application(application_config)
    {
        const auto pbr_scene_init_script_index = scripting::ScriptManager::instance().create_script(scripting::Script{
            .script_name = "init_pbr_script",
            .script_path = wstring_to_string(
                core::FileSystem::instance().get_absolute_path(L"game/sandbox/scripts/init_pbr_scene.lua")),
        });

        auto default_scene = scene::Scene("Default Scene", pbr_scene_init_script_index);

        default_scene.add_light(interop::Light{
            .light_type = interop::LightType::Point,
            .world_space_position_or_direction = math::XMFLOAT3{-10.0f, 0.0f, 0.0f},
            .color = math::XMFLOAT3{1.0f, 1.0f, 1.0f},
            .intensity = 1.0f,
            .scale_or_sun_angle = 0.2f,
        });

        default_scene.add_light(interop::Light{
            .light_type = interop::LightType::Point,
            .world_space_position_or_direction = math::XMFLOAT3{10.0f, 0.0f, 0.0f},
            .color = math::XMFLOAT3{1.0f, 1.0f, 0.0f},
            .intensity = 1.0f,
            .scale_or_sun_angle = 0.2f,
        });

        scene::SceneManager::instance().add_scene(std::move(default_scene));
    }

    virtual void update(const float delta_time) override
    {
        // Make the sun bounce from one side of the horizon to another for visualization purposes.
        auto &current_scene_light_buffer =
            scene::SceneManager::instance().get_current_scene().get_lights().get_light_buffer();

        // static auto increment_direction = -1.0f;
        //
        // if (current_scene_light_buffer.sun_angle >= 0.0f)
        //{
        //     increment_direction = -1.0f;
        // }
        // else if (current_scene_light_buffer.sun_angle <= -180.0f)
        //{
        //     increment_direction = 1.0f;
        // }
        //
        // current_scene_light_buffer.sun_angle += delta_time * 0.04f * increment_direction;
        // current_scene_light_buffer.sun_angle = std::clamp(current_scene_light_buffer.sun_angle, -180.0f, 0.0f);
        //
        const auto projection_matrix = math::XMMatrixPerspectiveFovLH(math::XMConvertToRadians(60.0f),
                                                                      m_window->get_aspect_ratio(), 0.1f, 1000.0f);

        scene::SceneManager::instance().get_current_scene().update(projection_matrix, delta_time, m_frame_count,
                                                                   m_input);

        renderer::Renderer::instance().update_renderpasses(m_frame_count);
    }
};

std::unique_ptr<serenity::core::Application> serenity::core::create_application()
{
    return std::make_unique<SandBox>(serenity::core::ApplicationConfig{
        .log_to_console = true,
        .log_to_file = true,
        .dimensions =
            Float2{
                .x = 100.0f,
                .y = 100.0f,
            },
    });
}