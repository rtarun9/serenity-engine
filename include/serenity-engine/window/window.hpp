#pragma once

#include "serenity-engine/core/input.hpp"

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
        explicit Window(const std::string_view title, const Uint2 dimension);
        ~Window();

        void poll_events(core::Input &input);

        // Returns a pair in form of {width, height}
        Uint2 get_dimensions() const
        {
            return m_dimension;
        }

        HWND get_window_handle() const
        {

            return m_window_handle;
        }

      private:
        SDL_Window *m_window{};
        HWND m_window_handle{};

        Uint2 m_dimension{};
    };
} // namespace serenity::window