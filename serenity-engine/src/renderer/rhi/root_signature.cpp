#include "serenity-engine/renderer/rhi/root_signature.hpp"

namespace serenity::renderer::rhi
{
    RootSignature::RootSignature(const comptr<ID3D12Device> &device)
    {
        // Create the bindless root signature with some static samplers and 64 32 bit root constants.
        auto root_signature_parameters = std::array<CD3DX12_ROOT_PARAMETER1, 1u>{};
        root_signature_parameters[0].InitAsConstants(RootSignature::NUM_32_BIT_ROOT_CONSTANTS, 0u, 0u);

        const auto anisotropic_sampler_desc = CD3DX12_STATIC_SAMPLER_DESC(0u);
        const auto linear_wrap_sampler_desc = CD3DX12_STATIC_SAMPLER_DESC(1u, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

        const auto sampler_descs = std::array{anisotropic_sampler_desc, linear_wrap_sampler_desc};

        const auto versioned_root_signature_desc =
            CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(static_cast<uint32_t>(root_signature_parameters.size()),
                                                  root_signature_parameters.data(), 2u, sampler_descs.data(),
                                                  D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
                                                      D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED);

        auto serialized_root_signature = comptr<ID3DBlob>{};
        throw_if_failed(::D3D12SerializeVersionedRootSignature(&versioned_root_signature_desc,
                                                               &serialized_root_signature, nullptr));

        throw_if_failed(device->CreateRootSignature(0u, serialized_root_signature->GetBufferPointer(),
                                                    serialized_root_signature->GetBufferSize(),
                                                    IID_PPV_ARGS(&m_root_signature)));

        core::Log::instance().info("Created bindless root signature");
    }

    RootSignature::~RootSignature()
    {
        core::Log::instance().info("Destroyed root signature");
    }
} // namespace serenity::renderer::rhi