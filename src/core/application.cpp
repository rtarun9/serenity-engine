#include "serenity-engine/core/application.hpp"

namespace serenity::core
{
    Application::Application()
    {
        m_log = std::make_unique<Log>();
        m_file_system = std::make_unique<FileSystem>();
    }

    void Application::run()
    {
    }
} // namespace serenity::core