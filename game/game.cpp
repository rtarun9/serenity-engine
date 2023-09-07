#include "serenity-engine/serenity-engine.hpp"

#include "shaders/interop/render_resources.hlsli"

using namespace serenity;

class Game final : public core::Application
{

  public:
    explicit Game()
    {
        auto default_scene = scene::Scene("Default Scene");
        default_scene.add_model("data/Cube/glTF/Cube.gltf", "Cube");

        scene::SceneManager::instance().add_scene(std::move(default_scene));
    }

    ~Game() = default;

    virtual void update(const float delta_time) override
    {
        // Update scene objects (camera and models).
        auto &current_scene_camera = scene::SceneManager::instance().get_current_scene().get_camera();
        current_scene_camera.update(delta_time, m_input);

        const auto window_dimensions = m_window->get_dimensions();
        const auto aspect_ratio = static_cast<float>(window_dimensions.x) / static_cast<float>(window_dimensions.y);

        const auto projection_matrix =
            math::XMMatrixPerspectiveFovLH(math::XMConvertToRadians(45.0f), aspect_ratio, 0.1f, 1000.0f);

        scene::SceneManager::instance().get_current_scene().update(projection_matrix);
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
    return std::make_unique<Game>();
}