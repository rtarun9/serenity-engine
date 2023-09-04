#pragma once

#include "serenity-engine/graphics/buffer.hpp"

namespace serenity::scene
{
    struct Mesh
    {
        graphics::Buffer position_buffer{};

        graphics::Buffer index_buffer{};

        uint32_t indices_count{};
    };
} // namespace serenity::scene