#include "serenity-engine/renderer/rhi/command_signature.hpp"

#include "serenity-engine/renderer/rhi/d3d_utils.hpp"
#include "serenity-engine/renderer/rhi/root_signature.hpp"

namespace serenity::renderer::rhi
{
    CommandSignature::CommandSignature(const comptr<ID3D12Device> &device)
    {
        // For now, there will be 2 indirect arguments.
        // 1. A single 32 bit root constant that describes the mesh id (primitive id to be more exact).
        // 2. The type (draw indexed).

        const auto indirect_argument_desc = std::array<D3D12_INDIRECT_ARGUMENT_DESC, 2u>{
            D3D12_INDIRECT_ARGUMENT_DESC{
                .Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT,
                .Constant =
                    {
                        .RootParameterIndex = 0u,
                        .DestOffsetIn32BitValues = 0u,
                        .Num32BitValuesToSet = 1u,
                    },
            },
            D3D12_INDIRECT_ARGUMENT_DESC{
                .Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED,
            },
        };

        const auto command_signature_desc = D3D12_COMMAND_SIGNATURE_DESC{
            .ByteStride = sizeof(IndirectCommandArgs),
            .NumArgumentDescs = static_cast<uint32_t>(indirect_argument_desc.size()),
            .pArgumentDescs = indirect_argument_desc.data(),
        };

        device->CreateCommandSignature(&command_signature_desc, RootSignature::instance().get_root_signature().Get(),
                                       IID_PPV_ARGS(&m_command_signature));

        set_name(m_command_signature.Get(), L"Command Signature");

        core::Log::instance().info("Created command signature");
    }
} // namespace serenity::renderer::rhi