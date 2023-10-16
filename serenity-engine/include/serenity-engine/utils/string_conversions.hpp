#pragma once

namespace serenity
{
    // Helper functions for converting strings from wstring to string and vice versa.
    // Reference :
    // https://github.com/turanszkij/WickedEngine/blob/bb519474cad797af78f53bbee622520efbb725f7/WickedEngine/wiHelper.cpp#L1520

    inline std::wstring string_to_wstring(const std::string_view input_string)
    {
        auto result = std::wstring{};
        const auto input = std::string(input_string);

        const auto length = ::MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, NULL, 0);
        if (length > 0)
        {
            result.resize(size_t(length) - 1);
            MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, result.data(), length);
        }

        return std::move(result);
    }

    inline std::string wstring_to_string(const std::wstring_view input_string)
    {
        auto result = std::string{};
        const auto input = std::wstring(input_string);

        const auto length = ::WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, NULL, 0, NULL, NULL);
        if (length > 0)
        {
            result.resize(size_t(length) - 1);
            WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, result.data(), length, NULL, NULL);
        }

        return std::move(result);
    }
} // namespace serenity