#include "serenity-engine/window/window.hpp"

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

namespace serenity::window
{
    Window::Window(const std::string_view title, const uint32_t width, const uint32_t height)
        : m_width(width), m_height(height)
    {
        if (width == 0 || height == 0)
        {
            core::Log::get().critical("Window dimensions cannot be 0");
        }

        // Initialize SDL3.
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            core::Log::get().critical("Failed to initialize SDL3");
        }

        m_window = SDL_CreateWindowWithPosition(title.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                                static_cast<int>(width), static_cast<int>(height), 0);

        if (!m_window)
        {
            core::Log::get().critical("Failed to create SDL window");
        }

        core::Log::get().info("Created window");
    }

    Window::~Window()
    {
        core::Log::get().info("Destroyed window");

        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    void Window::poll_events(core::Input &input)
    {
        SDL_Event event{};
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                input.quit = true;
            }

            const auto *keyboard_state = SDL_GetKeyboardState(nullptr);

            std::fill(input.key_states.begin(), input.key_states.end(), false);

            if (keyboard_state[SDL_SCANCODE_W])
            {
                input.set_key_state(core::Keys::W, true);
            }

            if (keyboard_state[SDL_SCANCODE_A])
            {
                input.set_key_state(core::Keys::A, true);
            }

            if (keyboard_state[SDL_SCANCODE_S])
            {
                input.set_key_state(core::Keys::S, true);
            }

            if (keyboard_state[SDL_SCANCODE_D])
            {
                input.set_key_state(core::Keys::D, true);
            }

            if (keyboard_state[SDL_SCANCODE_SPACE])
            {
                input.set_key_state(core::Keys::Space, true);
            }

            if (keyboard_state[SDL_SCANCODE_UP])
            {
                input.set_key_state(core::Keys::ArrowUp, true);
            }

            if (keyboard_state[SDL_SCANCODE_LEFT])
            {
                input.set_key_state(core::Keys::ArrowLeft, true);
            }

            if (keyboard_state[SDL_SCANCODE_DOWN])
            {
                input.set_key_state(core::Keys::ArrowDown, true);
            }

            if (keyboard_state[SDL_SCANCODE_RIGHT])
            {
                input.set_key_state(core::Keys::ArrowRight, true);
            }
        }
    }
} // namespace serenity::window