#include "serenity-engine/core/application.hpp"

namespace serenity::core
{
    Application::Application()
    {
        m_log = std::make_unique<Log>();
        m_file_system = std::make_unique<FileSystem>();

        m_window = std::make_unique<window::Window>("serenity-engine", 1080u, 720u);
    }

    void Application::run()
    {
        auto quit = false;
        while (!quit)
        {
            m_window->poll_events(m_input);

            if (m_input.quit == true)
            {
                quit = true;
            }
        }
    }
} // namespace serenity::core