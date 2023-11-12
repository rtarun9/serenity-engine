#include "serenity-engine/window/window.hpp"

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_syswm.h>

namespace serenity::window
{
    Window::Window(const Float2 screen_percent_to_cover)
    {
        if (screen_percent_to_cover.x == 0.0f || screen_percent_to_cover.y == 0.0f)
        {
            core::Log::instance().critical("Window dimensions cannot be 0");
        }

        // Initialize SDL3.
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            core::Log::instance().critical("Failed to initialize SDL3");
        }

        // Get monitor dimensions.
        auto display_mode = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());

        m_dimension = {
            .x = static_cast<uint32_t>(display_mode->w * screen_percent_to_cover.x / 100.0f),
            .y = static_cast<uint32_t>(display_mode->h * screen_percent_to_cover.y / 100.0f),
        };

        create_sdl3_window();
    }

    Window::Window(const Uint2 dimension) : m_dimension(dimension)
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

        create_sdl3_window();
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

        auto &keyboard = input.keyboard;
        auto &mouse = input.mouse;

        while (SDL_PollEvent(&event))
        {
            const auto *keyboard_state = SDL_GetKeyboardState(nullptr);
            std::fill(keyboard.key_states.begin(), keyboard.key_states.end(), false);

            for (const auto &callback : m_event_callbacks)
            {
                if (callback(Event{.internal_event = event}) == EventType::SkipNextEvents)
                {
                    return;
                }
            }

            if (keyboard_state[SDL_SCANCODE_W])
            {
                keyboard.set_key_state(core::Keys::W, true);
            }

            if (keyboard_state[SDL_SCANCODE_A])
            {
                keyboard.set_key_state(core::Keys::A, true);
            }

            if (keyboard_state[SDL_SCANCODE_S])
            {
                keyboard.set_key_state(core::Keys::S, true);
            }

            if (keyboard_state[SDL_SCANCODE_D])
            {
                keyboard.set_key_state(core::Keys::D, true);
            }

            if (keyboard_state[SDL_SCANCODE_SPACE])
            {
                keyboard.set_key_state(core::Keys::Space, true);
            }

            if (keyboard_state[SDL_SCANCODE_ESCAPE])
            {
                keyboard.set_key_state(core::Keys::Escape, true);
            }

            if (keyboard_state[SDL_SCANCODE_UP])
            {
                keyboard.set_key_state(core::Keys::ArrowUp, true);
            }

            if (keyboard_state[SDL_SCANCODE_LEFT])
            {
                keyboard.set_key_state(core::Keys::ArrowLeft, true);
            }

            if (keyboard_state[SDL_SCANCODE_DOWN])
            {
                keyboard.set_key_state(core::Keys::ArrowDown, true);
            }

            if (keyboard_state[SDL_SCANCODE_RIGHT])
            {
                keyboard.set_key_state(core::Keys::ArrowRight, true);
            }

            switch (event.type)
            {
            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mouse.left_button_down = true;
                    auto mouse_pos_x = 0.0f;
                    auto mouse_pos_y = 0.0f;

                    SDL_GetMouseState(&mouse_pos_x, &mouse_pos_y);

                    mouse.mouse_position = Float2{
                        .x = mouse_pos_x,
                        .y = mouse_pos_y,
                    };
                }
            }
            break;

            case SDL_EVENT_MOUSE_BUTTON_UP: {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mouse.left_button_down = false;
                }
            }
            break;

            // Handle window movement (for borderless window).
            case SDL_EVENT_MOUSE_MOTION: {
                // Move the window only when left button is down and the space key is pressed.
                // Logic : The position of mouse when left button was pressed acts as the 'origin'. The window is moved
                // in the same direction as the direction vector between the new mouse position and teh old one.
                if (mouse.left_button_down && keyboard.is_key_pressed(core::Keys::Space))
                {
                    auto mouse_pos_x = 0.0f;
                    auto mouse_pos_y = 0.0f;

                    SDL_GetMouseState(&mouse_pos_x, &mouse_pos_y);

                    const auto dx = static_cast<int>(mouse_pos_x - mouse.mouse_position.x);
                    const auto dy = static_cast<int>(mouse_pos_y - mouse.mouse_position.y);

                    auto window_pos_x = static_cast<int>(0);
                    auto window_pos_y = static_cast<int>(0);

                    SDL_GetWindowPosition(m_window, &window_pos_x, &window_pos_y);
                    SDL_SetWindowPosition(m_window, window_pos_x + dx, window_pos_y + dy);
                }
            }
            break;
            }
        }
    }

    void Window::add_event_callback(std::function<EventType(Event)> callback)
    {
        m_event_callbacks.push_back({callback});
    }

    void Window::create_sdl3_window()
    {
        m_window = SDL_CreateWindowWithPosition("serenity-engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                                static_cast<int>(m_dimension.x), static_cast<int>(m_dimension.y), 0);
        SDL_SetWindowBordered(m_window, SDL_FALSE);

        if (!m_window)
        {
            core::Log::instance().critical("Failed to create SDL window");
        }

        // Get the underlying OS window handle.
        auto window_info = SDL_SysWMinfo{};
        SDL_GetWindowWMInfo(m_window, &window_info, SDL_SYSWM_CURRENT_VERSION);

        m_window_handle = window_info.info.win.window;

        core::Log::instance().info("Created window");
    }
} // namespace serenity::window