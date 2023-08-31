#pragma once

#include "d3d_utils.hpp"

namespace serenity::graphics
{
    // Swapchain holds / implements surfaces that we can render into and store before presenting it to output.
    // Handles swapping and allocation of backbuffers.
    class Swapchain
    {
      public:
        explicit Swapchain(IDXGIFactory5 *const factory, ID3D12CommandQueue *const direct_command_queue,
                           const uint32_t width, const uint32_t height, const HWND window_handle);
        ~Swapchain();

      public:
        static constexpr auto NUM_BACK_BUFFERS = 3u;

      private:
        Swapchain(const Swapchain &other) = delete;
        Swapchain &operator=(const Swapchain &other) = delete;

        Swapchain(Swapchain &&other) = delete;
        Swapchain &operator=(Swapchain &&other) = delete;

      private:
        comptr<IDXGISwapChain2> m_swapchain{};
    };
} // namespace serenity::graphics