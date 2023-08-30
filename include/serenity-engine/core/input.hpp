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
        Total
    };

    struct Input
    {
        // True indicates key is pressed, false indicates it is not.
        std::array<bool, get_enum_class_value(Keys::Total)> key_states{};

        bool quit{false};

        bool is_key_pressed(const Keys key)
        {
            return key_states.at(get_enum_class_value(key));
        }

        void set_key_state(const Keys key, const bool state)
        {
            key_states.at(get_enum_class_value(key)) = state;
        }
    };
} // namespace serenity::core