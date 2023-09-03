#pragma once

#include "d3d_utils.hpp"

#include "descriptor_heap.hpp"

#include "serenity-engine/core/singleton_instance.hpp"

namespace serenity::graphics
{
    // Backbuffer abstraction that ties together the descriptor handle and resource.
    struct Backbuffer
    {
        comptr<ID3D12Resource> resource{};
        DescriptorHandle descriptor_handle{};
    };

    // Swapchain holds / implements surfaces that we can render into and store before presenting it to output.
    // Handles swapping and allocation of backbuffers.
    // NOTE : As a design choice, the swapchain will be a SingletonInstance, but will be created by device.
    class Swapchain final : public core::SingletonInstance<Swapchain>
    {
      public:
        explicit Swapchain(const comptr<IDXGIFactory5> &factory, const comptr<ID3D12Device5> &device,
                           const comptr<ID3D12CommandQueue> &direct_command_queue,
                           const DescriptorHeap &rtv_descriptor_heap, const Uint2 dimension, const HWND window_handle,
                           bool enable_vsync = true);
        ~Swapchain();

        uint32_t get_current_backbuffer_index() const
        {
            return m_current_backbuffer_index;
        }

        Backbuffer &get_current_back_buffer()
        {
            return m_backbuffers.at(m_current_backbuffer_index);
        }

        D3D12_VIEWPORT get_viewport() const
        {
            return m_viewport;
        }

        D3D12_RECT get_scissor_rect() const
        {
            return m_scissor_rect;
        }

        // The m_current_backbuffer_index variable will be updated after the swapchain::present function is called.
        void present();

      private:
        void check_for_tearing_support(const comptr<IDXGIFactory5> &factory);
        void update_rtvs(const comptr<ID3D12Device> &device, const DescriptorHeap &rtv_descriptor_heap,
                         const Uint2 dimension);

      private:
        Swapchain(const Swapchain &other) = delete;
        Swapchain &operator=(const Swapchain &other) = delete;

        Swapchain(Swapchain &&other) = delete;
        Swapchain &operator=(Swapchain &&other) = delete;

      public:
        static constexpr auto NUM_BACK_BUFFERS = 3u;

      private:
        comptr<IDXGISwapChain4> m_swapchain{};

        uint32_t m_current_backbuffer_index{};

        D3D12_VIEWPORT m_viewport{};
        D3D12_RECT m_scissor_rect{};

        std::array<Backbuffer, NUM_BACK_BUFFERS> m_backbuffers{};

        bool m_vsync_enabled{true};
        bool m_tearing_supported{false};
    };
} // namespace serenity::graphics