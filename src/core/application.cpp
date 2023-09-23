#include "serenity-engine/core/application.hpp"

#include "serenity-engine/utils/timer.hpp"

namespace serenity::core
{
    Application::Application(const ApplicationConfig &application_config)
    {
        // Create the engine subsystems.
        m_log = std::make_unique<Log>(application_config.log_to_console, application_config.log_to_console);

        m_file_system = std::make_unique<FileSystem>();

        if (const auto window_dimensions = std::get_if<Uint2>(&application_config.dimensions); window_dimensions)
        {
            m_window = std::make_unique<window::Window>(*window_dimensions);
        }
        else if (const auto screen_percent_to_cover = std::get_if<Float2>(&application_config.dimensions);
                 screen_percent_to_cover)
        {
            m_window = std::make_unique<window::Window>(*screen_percent_to_cover);
        }

        m_renderer = std::make_unique<renderer::Renderer>(*(m_window.get()));

        m_scene_manager = std::make_unique<scene::SceneManager>();

        m_editor = std::make_unique<editor::Editor>(*(m_window.get()));
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

            const auto delta_time = timer.get_delta_time();
            update(delta_time);

            render();

            timer.tick();
        }
    }

    void Application::render()
    {
        renderer::Renderer::instance().render();

        ++m_frame_count;
    }
} // namespace serenity::core