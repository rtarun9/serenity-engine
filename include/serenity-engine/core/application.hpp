#pragma once

#include "file_system.hpp"
#include "log.hpp"

namespace serenity::core
{
    // All serenity engine application's must inherit from the class application.
    class Application
    {
      public:
        Application();
        virtual ~Application() = default;

        virtual void run() final;

      protected:
        Application(const Application &other) = delete;
        Application &operator=(const Application &other) = delete;

        Application(Application &&other) = delete;
        Application &operator=(Application &&other) = delete;

      protected:
        std::unique_ptr<Log> m_log{};
        std::unique_ptr<FileSystem> m_file_system{};
    };
} // namespace serenity::core