#pragma once

#include "serenity-engine/core/input.hpp"

#include "SDL_events.h"

// Forward declarations.
struct SDL_Window;

namespace serenity::window
{
    // Used for event callbacks.
    struct Event
    {
        SDL_Event internal_event{};
    };

    // A light weight abstraction over SDL3's window.
    // The poll_events function takes in a reference to a Input object.
    // This design is chosen because events / input is closely related to the window (while the relationship is not
    // entirely obvious in SDL3, it is in GLFW and Win32).
    class Window
    {
      public:
        // Call this constructor to set the window dimensions to a user set percentage of the monitor.
        explicit Window(const Float2 screen_percent_to_cover);

        // Call this constructor to set the window dimensions to a user set window dimension.
        explicit Window(const Uint2 dimension);

        ~Window();

        void poll_events(core::Input &input);

        SDL_Window *get_internal_window() const { return m_window; }

        float get_aspect_ratio() const { return static_cast<float>(m_dimension.x) / static_cast<float>(m_dimension.y); }

        Uint2 get_dimensions() const { return m_dimension; }

        HWND get_window_handle() const { return m_window_handle; }

        // Experimental : If you want to have event handling (such as for editor), you can pass the event handlers here.
        // Expected format : [&]() {... A function that takes in as input Event (SDL_Event internally) ...}
        void add_event_callback(std::function<void(Event)> callback);

      private:
        // Function that has shared logic by all types of constructors for window creation (assumes that m_dimension is
        // set by the constructors and SDL3 is initialized).
        void create_sdl3_window();

      private:
        Window(const Window &other) = delete;
        Window &operator=(const Window &other) = delete;

        Window(Window &&other) = delete;
        Window &operator=(Window &&other) = delete;

      private:
        SDL_Window *m_window{};
        HWND m_window_handle{};

        Uint2 m_dimension{};

        std::vector<std::function<void(Event)>> m_event_callbacks{};
    };
} // namespace serenity::window