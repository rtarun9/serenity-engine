#include "serenity-engine/graphics/swapchain.hpp"

namespace serenity::graphics
{
    Swapchain::Swapchain(IDXGIFactory5 *const factory, ID3D12CommandQueue *const direct_command_queue,
                         const uint32_t width, const uint32_t height, const HWND window_handle)
    {
        // Create the swapchain.
        const auto swapchain_desc = DXGI_SWAP_CHAIN_DESC1{
            .Width = width,
            .Height = height,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc =
                {
                    1u,
                    0u,
                },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = Swapchain::NUM_BACK_BUFFERS,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        };

        comptr<IDXGISwapChain1> swapchain{};
        throw_if_failed(factory->CreateSwapChainForHwnd(direct_command_queue, window_handle, &swapchain_desc, nullptr,
                                                        nullptr, &swapchain));
        throw_if_failed(swapchain.As(&m_swapchain));

        core::Log::get().info("Created swapchain");
    }

    Swapchain::~Swapchain()
    {
        core::Log::get().info("Destroyed swapchain");
    }

} // namespace serenity::graphics