#pragma once

#include "file_system.hpp"
#include "log.hpp"
#include "input.hpp"

#include "serenity-engine/window/window.hpp"
#include "serenity-engine/graphics/device.hpp"

namespace serenity::core
{
    // All serenity engine application's must inherit from the class application.
    // NOTE : Implement the 'create_application' function that returns a unique_ptr of of derived class of Application.
    class Application
    {
      public:
        explicit Application();
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

        Input m_input{};
        std::unique_ptr<window::Window> m_window{};

        std::unique_ptr<graphics::Device> m_graphics_device{};
    };

    // To be implemented in only a single class that inherits from Application.
    extern std::unique_ptr<Application> create_application();

} // namespace serenity::core