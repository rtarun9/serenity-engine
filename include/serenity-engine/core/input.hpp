#pragma once

#include "serenity-engine/utils/enum_value.hpp"

namespace serenity::core
{
    enum class Keys : uint8_t
    {
        W,
        A,
        S,
        D,
        Space,
        Escape,
        ArrowUp,
        ArrowLeft,
        ArrowDown,
        ArrowRight,
        Count
    };

    struct KeyboardState
    {
        // True indicates key is pressed, false indicates it is not.
        std::array<bool, get_enum_class_value(Keys::Count)> key_states{};

        bool is_key_pressed(const Keys key)
        {
            return key_states.at(get_enum_class_value(key));
        }

        void set_key_state(const Keys key, const bool state)
        {
            key_states.at(get_enum_class_value(key)) = state;
        }
    };

    struct MouseState
    {
        bool left_button_down{false};
        bool right_button_down{false};

        Float2 mouse_position{};
    };

    struct Input
    {
        KeyboardState keyboard{};
        MouseState mouse{};
    };
} // namespace serenity::core