#pragma once

namespace serenity
{
    // Collection of datatypes and values used throughout the engine.
    struct Uint2
    {
        uint32_t x{};
        uint32_t y{};
    };

    struct Uint3
    {
        uint32_t x{};
        uint32_t y{};
        uint32_t z{};
    };

    struct Float2
    {
        float x{};
        float y{};
    };

    static constexpr uint32_t INVALID_INDEX_U32 = -1u;

} // namespace serenity