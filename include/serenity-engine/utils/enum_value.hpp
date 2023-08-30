#pragma once

namespace serenity
{
    // Collection of helper functions (related to enum classes and enums).
    template <typename T> constexpr std::underlying_type<T>::type get_enum_class_value(const T key)
    {
        return static_cast<std::underlying_type<T>::type>(key);
    }
} // namespace serenity