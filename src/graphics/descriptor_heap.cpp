#include "serenity-engine/graphics/descriptor_heap.hpp"

namespace serenity::graphics
{
    DescriptorHeap::DescriptorHeap(ID3D12Device *const device, const D3D12_DESCRIPTOR_HEAP_TYPE descriptor_heap_type,
                                   const uint32_t num_descriptors)
        : m_descriptor_heap_type(descriptor_heap_type)
    {
        // Create the descriptor heap.
        const auto descriptor_heap_flags = (descriptor_heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ||
                                            descriptor_heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
                                               ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
                                               : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        const auto descriptor_heap_desc = D3D12_DESCRIPTOR_HEAP_DESC{
            .Type = descriptor_heap_type,
            .NumDescriptors = num_descriptors,
            .Flags = descriptor_heap_flags,
            .NodeMask = 0u,
        };

        throw_if_failed(device->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&m_descriptor_heap)));

        // Set name to descriptor heap.
        set_name(m_descriptor_heap.Get(), descriptor_heap_type_to_string(descriptor_heap_type) + L" Descriptor Heap");

        core::Log::get().info(std::format("Created descriptor heap of type {}",
                                          wstring_to_string(descriptor_heap_type_to_string(descriptor_heap_type))));
    }

    DescriptorHeap::~DescriptorHeap()
    {
        core::Log::get().info(std::format("Destroyed descriptor heap of type {}",
                                          wstring_to_string(descriptor_heap_type_to_string(m_descriptor_heap_type))));
    }
} // namespace serenity::graphics