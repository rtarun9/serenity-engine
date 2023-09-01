#include "serenity-engine/window/window.hpp"

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_syswm.h>

namespace serenity::window
{
    Window::Window(const std::string_view title, const Uint2 dimension) : m_dimension(dimension)
    {
        if (dimension.x == 0 || dimension.y == 0)
        {
            core::Log::instance().critical("Window dimensions cannot be 0");
        }

        // Initialize SDL3.
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            core::Log::instance().critical("Failed to initialize SDL3");
        }

        m_window = SDL_CreateWindowWithPosition(title.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                                static_cast<int>(dimension.x), static_cast<int>(dimension.y), 0);

        if (!m_window)
        {
            core::Log::instance().critical("Failed to create SDL window");
        }

        // Get the underlying OS window handle.
        SDL_SysWMinfo window_info{};
        SDL_GetWindowWMInfo(m_window, &window_info, SDL_SYSWM_CURRENT_VERSION);

        m_window_handle = window_info.info.win.window;

        core::Log::instance().info("Created window");
    }

    Window::~Window()
    {
        core::Log::instance().info("Destroyed window");

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

            if (keyboard_state[SDL_SCANCODE_ESCAPE])
            {
                input.set_key_state(core::Keys::Escape, true);
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