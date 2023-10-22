#pragma once

#include "d3d_utils.hpp"

namespace serenity::renderer::rhi
{
    // CommandSignature is used to specify the indirect argument buffer format, the command type that will be used (i.e
    // draw instanced, draw indexed instanced or dispatch), and the per command call resource bindings.
    // At run time, command list will have to interpret the contents of indirect argument buffer according to the format
    // specified by command signature.
    struct CommandSignature
    {
        comptr<ID3D12CommandSignature> m_command_signature{};
     
        // Keep track of the number of draw calls.
        uint32_t m_draw_structure_count{};
    };
} // namespace serenity::renderer::rhi