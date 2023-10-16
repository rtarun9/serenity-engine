#include "serenity-engine/renderer/rhi/descriptor_heap.hpp"

namespace serenity::renderer::rhi
{
    void DescriptorHandle::offset(const uint32_t count)
    {
        cpu_descriptor_handle.ptr += static_cast<size_t>(count) * descriptor_size;
        gpu_descriptor_handle.ptr += static_cast<uint64_t>(count) * descriptor_size;

        index += count;
    }

    DescriptorHeap::DescriptorHeap(const comptr<ID3D12Device> &device,
                                   const D3D12_DESCRIPTOR_HEAP_TYPE descriptor_heap_type,
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
        set_name(m_descriptor_heap.Get(), descriptor_heap_type_to_wstring(descriptor_heap_type) + L" Descriptor Heap");

        m_descriptor_size = device->GetDescriptorHandleIncrementSize(descriptor_heap_type);

        // Setup descriptor handle for heap start + current descriptor handle.
        m_descriptor_handle_for_start = DescriptorHandle{
            .cpu_descriptor_handle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart(),
            .gpu_descriptor_handle = (descriptor_heap_flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
                                         ? m_descriptor_heap->GetGPUDescriptorHandleForHeapStart()
                                         : D3D12_GPU_DESCRIPTOR_HANDLE{},
            .descriptor_size = m_descriptor_size,
            .index = 0u,
        };

        m_current_descriptor_handle = m_descriptor_handle_for_start;

        core::Log::instance().info(
            std::format("Created descriptor heap of type {}", descriptor_heap_type_to_string(descriptor_heap_type)));
    }

    DescriptorHeap::~DescriptorHeap()
    {
        core::Log::instance().info(std::format("Destroyed descriptor heap of type {}",
                                               descriptor_heap_type_to_string(m_descriptor_heap_type)));
    }

    uint32_t DescriptorHeap::get_descriptor_index(const DescriptorHandle &descriptor_handle) const
    {
        return static_cast<uint32_t>(
            (descriptor_handle.cpu_descriptor_handle.ptr - m_descriptor_handle_for_start.cpu_descriptor_handle.ptr) /
            m_descriptor_size);
    }

    DescriptorHandle DescriptorHeap::get_handle_at_index(const uint32_t index) const
    {
        auto descriptor_handle = m_descriptor_handle_for_start;
        descriptor_handle.offset(index);

        return descriptor_handle;
    }

    void DescriptorHeap::offset_current_handle(const uint32_t offset)
    {
        m_current_descriptor_handle.offset(offset);
    }
} // namespace serenity::renderer::rhi
