#pragma once

#include "file_system.hpp"
#include "input.hpp"
#include "log.hpp"

#include "serenity-engine/graphics/device.hpp"
#include "serenity-engine/window/window.hpp"

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

        // To be implemented by applications inheriting from this class.
        virtual void update(const float delta_time) = 0;
        virtual void render() = 0;

      private:
        Application(const Application &other) = delete;
        Application &operator=(const Application &other) = delete;

        Application(Application &&other) = delete;
        Application &operator=(Application &&other) = delete;

        // Classes that inherit SingletonInstance can be accessed using T::get(), so there is no need to make the
        // protected / public.
      private:
        std::unique_ptr<Log> m_log{};
        std::unique_ptr<FileSystem> m_file_system{};
        std::unique_ptr<graphics::Device> m_graphics_device{};

      public:
        Input m_input{};
        std::unique_ptr<window::Window> m_window{};

        // The number of frames rendered.
        uint32_t m_frame_index{};
    };

    // To be implemented in only a single class that inherits from Application.
    extern std::unique_ptr<Application> create_application();

} // namespace serenity::core