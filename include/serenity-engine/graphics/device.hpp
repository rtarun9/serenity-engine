#pragma once

#include "command_list.hpp"
#include "command_queue.hpp"
#include "descriptor_heap.hpp"
#include "swapchain.hpp"

#include "d3d_utils.hpp"

namespace serenity::graphics
{
    // Abstraction for creating / destroying various graphics resources.
    // Encapsulates most renderer resources / objects in use : the swap chain, descriptor heaps, command queue's, etc.
    class Device
    {
      public:
        explicit Device(const HWND window_handle, const uint32_t width, const uint32_t height);
        ~Device();

      private:
        Device(const Device &other) = delete;
        Device &operator=(const Device &other) = delete;

        Device(Device &&other) = delete;
        Device &operator=(Device &&other) = delete;

      private:
        // DXGI factory is used for enumeration of adapters and other dxgi objects.
        comptr<IDXGIFactory6> m_factory{};

        // Interface to the d3d12 debug layer.
        comptr<ID3D12Debug5> m_debug_layer{};

        // Adpater represents the display subsystem (the actual GPU, video memory, etc).
        comptr<IDXGIAdapter4> m_adapter{};

        // Device is used for creation of most d3d12 objects. Represents a virtual adapter.
        comptr<ID3D12Device5> m_device{};

        // Command queues provide interface to submit and execute recorded commands on the gpu.
        std::unique_ptr<CommandQueue> m_direct_command_queue{};

        // Command list are used for recording gpu commands that are submitted for execution to the command queue.
        std::unique_ptr<CommandList> m_direct_command_list{};

        // Descriptor heaps are contiguous memory allocations of descriptors (which in turn are small blocks of memory
        // that fully describe a resource to the gpu).
        std::unique_ptr<DescriptorHeap> m_rtv_descriptor_heap{};

        // Swapchain implements / holds surfaces that we can render onto / store for presentation.
        // handles swapping of buffers.
        std::unique_ptr<Swapchain> m_swapchain{};
    };
} // namespace serenity::graphics