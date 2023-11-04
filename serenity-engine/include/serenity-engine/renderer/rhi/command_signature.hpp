#pragma once

#include "d3d_utils.hpp"

namespace serenity::renderer::rhi
{
    // References :
    // https://learn.microsoft.com/en-us/windows/win32/direct3d12/indirect-drawing
    // https://github.com/adepke/VanguardEngine/blob/437bb8abc5dd510498d445fe7facf0d428686078/VanguardEngine/Source/Rendering/Renderer.cpp#L331

    struct IndirectCommandArgs
    {
        uint32_t mesh_id;
        D3D12_DRAW_INDEXED_ARGUMENTS draw_arguments{};
    };

    // CommandSignature is used to specify the indirect argument buffer format, the command type that will be used (i.e
    // draw instanced, draw indexed instanced or dispatch), and the per command call resource bindings.
    // At run time, command list will have to interpret the contents of indirect argument buffer according to the format
    // specified by command signature.
    struct CommandSignature
    {
        explicit CommandSignature(const comptr<ID3D12Device> &device);
        comptr<ID3D12CommandSignature> m_command_signature{};
    };
} // namespace serenity::renderer::rhi