#pragma once

#include "file_system.hpp"
#include "input.hpp"
#include "log.hpp"

#include "serenity-engine/scene/scene_manager.hpp"

#include "serenity-engine/renderer/renderer.hpp"
#include "serenity-engine/window/window.hpp"

#include "serenity-engine/editor/editor.hpp"

#include "serenity-engine/scripting/script_manager.hpp"

namespace serenity::core
{
    struct ApplicationConfig
    {
        bool log_to_console{true};
        bool log_to_file{true};
        std::variant<Uint2, Float2> dimensions{};
    };

    // All serenity engine application's must inherit from this Application abstract class.
    // NOTE : Implement the 'create_application()' function that returns a unique_ptr of  derived class of
    // Application. The entry point / main function will use the create_application() function to get an object of
    // Application (a derived class that is) and call the run method.
    class Application
    {
      public:
        explicit Application(const ApplicationConfig &application_config);
        virtual ~Application() = default;

        // The run method should not be overridden.
        virtual void run() final;

        // To be implemented by applications inheriting from this class.
        virtual void update(const float delta_time) = 0;
        virtual void render() final;

      private:
        Application(const Application &other) = delete;
        Application &operator=(const Application &other) = delete;

        Application(Application &&other) = delete;
        Application &operator=(Application &&other) = delete;

        // Classes that inherit SingletonInstance can be accessed using T::instance(), so there is no need to make the
        // protected / public.
      private:
        std::unique_ptr<Log> m_log{};
        std::unique_ptr<FileSystem> m_file_system{};

        std::unique_ptr<renderer::Renderer> m_renderer{};

        std::unique_ptr<scene::SceneManager> m_scene_manager{};

        std::unique_ptr<editor::Editor> m_editor{};

        std::unique_ptr<scripting::ScriptManager> m_script_manager{};

      protected:
        Input m_input{};
        std::unique_ptr<window::Window> m_window{};

        // The number of frames rendered.
        uint32_t m_frame_count{};
    };

    // To be implemented in only a single class that inherits from Application.
    extern [[nodiscard]] std::unique_ptr<Application> create_application();

} // namespace serenity::core