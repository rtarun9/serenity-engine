#pragma once

#include "serenity-engine/core/input.hpp"

#include "SDL_events.h"

// Forward declarations.
struct SDL_Window;

namespace serenity::window
{
    // A light weight abstraction over SDL3's window.
    // The poll_events function takes in a reference to a Input object.
    // This design is chosen because events / input is closely related to the window.
    class Window
    {
      public:
        // Call this constructor to set the window dimensions to a user set percentage of the monitor.
        explicit Window(const std::string_view title, const Float2 screen_percent_to_cover);

        // Call this constructor to set the window dimensions to a user set window dimension.
        explicit Window(const std::string_view title, const Uint2 dimension);
        ~Window();

        void poll_events(core::Input &input);

        SDL_Window *get_internal_window() const
        {
            return m_window;
        }

        // Returns a pair in form of {width, height}
        Uint2 get_dimensions() const
        {
            return m_dimension;
        }

        HWND get_window_handle() const
        {
            return m_window_handle;
        }

        // Experimental : If you want to have event handling (such as for editor), you can pass the event handlers here.
        void add_event_callback(std::function<void(SDL_Event)> callback);

      private:
        SDL_Window *m_window{};
        HWND m_window_handle{};

        Uint2 m_dimension{};

        std::vector<std::function<void(SDL_Event)>> m_event_callbacks{};
    };
} // namespace serenity::window