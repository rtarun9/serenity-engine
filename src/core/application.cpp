#include "serenity-engine/core/application.hpp"

#include "serenity-engine/utils/timer.hpp"

namespace serenity::core
{
    Application::Application()
    {
        // Create the engine subsystems.
        m_log = std::make_unique<Log>();

        m_file_system = std::make_unique<FileSystem>();

        const auto window_dimension = Uint2{
            .x = 1080u,
            .y = 720u,
        };

        m_window = std::make_unique<window::Window>("serenity-engine", window_dimension);

        m_graphics_device = std::make_unique<graphics::Device>(m_window->get_window_handle(), window_dimension);

        m_mesh_loader = std::make_unique<asset::MeshLoader>();
    }

    void Application::run()
    {
        auto timer = Timer();

        auto quit = false;
        while (!quit)
        {
            m_window->poll_events(m_input);

            if (m_input.keyboard.is_key_pressed(Keys::Escape))
            {
                quit = true;
            }

            timer.tick();
            const auto delta_time = timer.get_delta_time();

            update(delta_time);
            render();
        }
    }
} // namespace serenity::core