#pragma once

#include "d3d_utils.hpp"

namespace serenity::graphics
{
    // A descriptor heap is a contiguous allocation of descriptors.
    // Descriptor is a small block of data that fully describes an object to the gpu.
    class DescriptorHeap
    {
      public:
        explicit DescriptorHeap(ID3D12Device *const device, const D3D12_DESCRIPTOR_HEAP_TYPE descriptor_heap_type,
                       const uint32_t num_descriptors);
        ~DescriptorHeap();

      private:
        DescriptorHeap(const DescriptorHeap &other) = delete;
        DescriptorHeap &operator=(const DescriptorHeap &other) = delete;

        DescriptorHeap(DescriptorHeap &&other) = delete;
        DescriptorHeap &operator=(DescriptorHeap &&other) = delete;

      private:
        comptr<ID3D12DescriptorHeap> m_descriptor_heap{};

        D3D12_DESCRIPTOR_HEAP_TYPE m_descriptor_heap_type{};
    };
} // namespace serenity::graphics