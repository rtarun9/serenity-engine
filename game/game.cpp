#include "serenity-engine/serenity-engine.hpp"

using namespace serenity;

class Game final : public core::Application
{

  public:
    explicit Game(const core::ApplicationConfig &application_config) : core::Application(application_config)
    {
        auto default_scene = scene::Scene("Default Scene");
        default_scene.add_model("data/Cube/glTF/Cube.gltf", "Cube");
        // default_scene.add_model("data/sketchfab_pbr_material_reference_chart/scene.gltf", "PBR_References");
        scene::SceneManager::instance().add_scene(std::move(default_scene));
    }

    ~Game() = default;

    virtual void update(const float delta_time) override
    {
        // Update scene objects (camera and models).
        auto &current_scene_camera = scene::SceneManager::instance().get_current_scene().get_camera();
        current_scene_camera.update(delta_time, m_input);

        // Make the sun bounce from one side of the horizon to another for visualization purposes.
        auto &current_scene_buffer = scene::SceneManager::instance().get_current_scene().get_scene_buffer();

        static float increment_direction = -1.0f;

        if (current_scene_buffer.sun_angle >= 0.0f)
        {
            increment_direction = -1.0f;
        }
        else if (current_scene_buffer.sun_angle <= -180.0f)
        {
            increment_direction = 1.0f;
        }

        current_scene_buffer.sun_angle += delta_time * 0.04f * increment_direction;
        current_scene_buffer.sun_angle = std::clamp(current_scene_buffer.sun_angle, -180.0f, 0.0f);

        const auto window_dimensions = m_window->get_dimensions();
        const auto aspect_ratio = static_cast<float>(window_dimensions.x) / static_cast<float>(window_dimensions.y);

        const auto projection_matrix =
            math::XMMatrixPerspectiveFovLH(math::XMConvertToRadians(60.0f), aspect_ratio, 0.1f, 1000.0f);

        scene::SceneManager::instance().get_current_scene().update(projection_matrix);

        renderer::Renderer::instance().update_renderpasses();
    }

    virtual void render() override
    {
        renderer::Renderer::instance().render();

        ++m_frame_count;
    }

  private:
};

std::unique_ptr<serenity::core::Application> serenity::core::create_application()
{
    return std::make_unique<Game>(serenity::core::ApplicationConfig{.log_to_console = true,
                                                                    .log_to_file = true,
                                                                    .dimensions = Float2{
                                                                        .x = 100.0f,
                                                                        .y = 100.0f,
                                                                    }});
}