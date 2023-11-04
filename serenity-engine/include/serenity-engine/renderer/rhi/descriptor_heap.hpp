#pragma once

#include "d3d_utils.hpp"

namespace serenity::renderer::rhi
{
    // Descriptor abstraction (holds both a CPU and GPU descriptor handle).
    // The GPU descriptor handle is to only be used when the descriptor heap is shader visible.
    struct DescriptorHandle
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor_handle{};
        D3D12_GPU_DESCRIPTOR_HANDLE gpu_descriptor_handle{};

        uint32_t descriptor_size{};

        uint32_t index{};

        void offset(const uint32_t count = 1u);
    };

    // A descriptor heap is a contiguous allocation of descriptors.
    // Descriptor is a small block of data that fully describes an object to the gpu.
    class DescriptorHeap
    {
      public:
        explicit DescriptorHeap(const comptr<ID3D12Device> &device,
                                const D3D12_DESCRIPTOR_HEAP_TYPE descriptor_heap_type, const uint32_t num_descriptors);
        ~DescriptorHeap();

        comptr<ID3D12DescriptorHeap> &get_descriptor_heap() { return m_descriptor_heap; }

        DescriptorHandle get_handle_for_heap_start() const { return m_descriptor_handle_for_start; }

        DescriptorHandle get_current_handle() const { return m_current_descriptor_handle; }

        uint32_t get_descriptor_index(const DescriptorHandle &descriptor_handle) const;

        DescriptorHandle get_handle_at_index(const uint32_t index) const;

        void offset_current_handle(const uint32_t offset = 1u);

      private:
        DescriptorHeap(const DescriptorHeap &other) = delete;
        DescriptorHeap &operator=(const DescriptorHeap &other) = delete;

        DescriptorHeap(DescriptorHeap &&other) = delete;
        DescriptorHeap &operator=(DescriptorHeap &&other) = delete;

      private:
        comptr<ID3D12DescriptorHeap> m_descriptor_heap{};

        uint32_t m_descriptor_size{};
        D3D12_DESCRIPTOR_HEAP_TYPE m_descriptor_heap_type{};

        DescriptorHandle m_descriptor_handle_for_start{};
        DescriptorHandle m_current_descriptor_handle{};
    };
} // namespace serenity::renderer::rhi