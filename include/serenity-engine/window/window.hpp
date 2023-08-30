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
        Window(const std::string_view title, const uint32_t width, const uint32_t height);
        ~Window();

        void poll_events(core::Input &input);

        // Returns a pair in form of {width, height}
        std::pair<uint32_t, uint32_t> get_dimensions() const
        {
            return {m_width, m_height};
        }

      private:
        SDL_Window *m_window{};

        uint32_t m_width{};
        uint32_t m_height{};
    };
} // namespace serenity::window