#include "serenity-engine/core/application.hpp"

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

        m_script_manager = std::make_unique<scripting::ScriptManager>();
    }

    void Application::run()
    {
        // note(rtarun9) : delta_time's units are milliseconds.
        auto start_time = std::chrono::high_resolution_clock::now();
        auto delta_time = 0.0f;

        auto quit = false;
        while (!quit)
        {
            m_window->poll_events(m_input);

            if (m_input.keyboard.is_key_pressed(Keys::Escape))
            {
                quit = true;
            }

            update(delta_time);
            render();

            const auto end_time = std::chrono::high_resolution_clock::now();
            delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            start_time = end_time;
        }
    }

    void Application::render()
    {
        renderer::Renderer::instance().render();

        ++m_frame_count;
    }
} // namespace serenity::core