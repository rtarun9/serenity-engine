#pragma once

#include "command_list.hpp"
#include "command_queue.hpp"
#include "descriptor_heap.hpp"
#include "swapchain.hpp"

#include "d3d_utils.hpp"

#include "serenity-engine/core/singleton_instance.hpp"

namespace serenity::graphics
{
    // Abstraction for creating / destroying various graphics resources.
    // Encapsulates most renderer resources / objects in use : the swap chain, descriptor heaps, command queue's, etc.
    class Device : public core::SingletonInstance<Device>
    {
      public:
        explicit Device(const HWND window_handle, const Uint2 dimensions);
        ~Device();

        // Resets the command buffer and allocator for the current frame.
        void frame_start();

        // Updates the frame fence value for current frame and updates current swapchain backbuffer index.
        void frame_end();

        CommandList &get_current_frame_direct_command_list() const
        {
            return *(m_direct_command_lists.at(m_current_swapchain_backbuffer_index).get());
        }

        CommandQueue &get_direct_command_queue()
        {
            return *(m_direct_command_queue.get());
        }

        uint64_t &get_frame_fence_value(const uint32_t frame_index)
        {
            return m_frame_fence_values.at(frame_index);
        }

      private:
        Device(const Device &other) = delete;
        Device &operator=(const Device &other) = delete;

        Device(Device &&other) = delete;
        Device &operator=(Device &&other) = delete;

      private:
        static constexpr uint32_t FRAMES_IN_FLIGHT = 3u;

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

        // Sync primitives.
        std::array<uint64_t, FRAMES_IN_FLIGHT> m_frame_fence_values{};

        // Command list are used for recording gpu commands that are submitted for execution to the command queue.
        // Using a unique command list per frame in flight.
        std::array<std::unique_ptr<CommandList>, FRAMES_IN_FLIGHT> m_direct_command_lists{};

        // Descriptor heaps are contiguous memory allocations of descriptors (which in turn are small blocks of memory
        // that fully describe a resource to the gpu).
        std::unique_ptr<DescriptorHeap> m_rtv_descriptor_heap{};

        // Swapchain implements / holds surfaces that we can render onto / store for presentation.
        // handles swapping of buffers.
        // This current swapchain back buffer index is updated in the frame_start() and frame_end() functions.
        std::unique_ptr<Swapchain> m_swapchain{};
        uint32_t m_current_swapchain_backbuffer_index{};
    };
} // namespace serenity::graphics