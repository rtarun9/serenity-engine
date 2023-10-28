#pragma once

#include "d3d_utils.hpp"

namespace serenity::renderer::rhi
{
    // Reference : https://learn.microsoft.com/en-us/windows/win32/direct3d12/indirect-drawing-and-gpu-culling-
    struct IndirectCommand
    {
        uint32_t object_id;
        D3D12_DRAW_INDEXED_ARGUMENTS draw_arguments{};
    };

    // CommandSignature is used to specify the indirect argument buffer format, the command type that will be used (i.e
    // draw instanced, draw indexed instanced or dispatch), and the per command call resource bindings.
    // At run time, command list will have to interpret the contents of indirect argument buffer according to the format
    // specified by command signature.
    struct CommandSignature
    {
        explicit CommandSignature(const comptr<ID3D12Device> &device);

        void append_indirect_command(const IndirectCommand &indirect_command)
        {
            m_indirect_commands.emplace_back(indirect_command);
        }

        comptr<ID3D12CommandSignature> m_command_signature{};

        // Each primitive appends to this vector.
        std::vector<IndirectCommand> m_indirect_commands{};
    };
} // namespace serenity::renderer::rhi