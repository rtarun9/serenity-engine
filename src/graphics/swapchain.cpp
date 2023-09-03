#include "serenity-engine/graphics/swapchain.hpp"

namespace serenity::graphics
{
    Swapchain::Swapchain(const comptr<IDXGIFactory5> &factory, const comptr<ID3D12Device5> &device,
                         const comptr<ID3D12CommandQueue> &direct_command_queue,
                         const DescriptorHeap &rtv_descriptor_heap, const Uint2 dimension, const HWND window_handle,
                         bool enable_vsync)
        : m_vsync_enabled(enable_vsync)
    {
        // Check for tearing support.
        check_for_tearing_support(factory);

        // Create the swapchain.
        const auto swapchain_desc = DXGI_SWAP_CHAIN_DESC1{
            .Width = dimension.x,
            .Height = dimension.y,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc =
                {
                    1u,
                    0u,
                },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = Swapchain::NUM_BACK_BUFFERS,
            .Scaling = DXGI_SCALING_STRETCH,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .Flags = (m_tearing_supported == true) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u,
        };

        auto swapchain = comptr<IDXGISwapChain1>{};

        throw_if_failed(factory->CreateSwapChainForHwnd(direct_command_queue.Get(), window_handle, &swapchain_desc,
                                                        nullptr, nullptr, &swapchain));
        throw_if_failed(swapchain.As(&m_swapchain));

        // Disable going to fullscreen mode when Alt + Enter is pressed. Transition to fullscreen will be handled
        // manually. Setup viewport and scissor rect.
        throw_if_failed(factory->MakeWindowAssociation(window_handle, DXGI_MWA_NO_ALT_ENTER));

        m_viewport = D3D12_VIEWPORT{
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<float>(dimension.x),
            .Height = static_cast<float>(dimension.y),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,

        };

        m_scissor_rect = D3D12_RECT{
            .left = 0u,
            .top = 0u,
            .right = static_cast<LONG>(dimension.x),
            .bottom = static_cast<LONG>(dimension.y),
        };

        update_rtvs(device, rtv_descriptor_heap, dimension);

        core::Log::instance().info("Created swapchain");
    }

    Swapchain::~Swapchain()
    {
        core::Log::instance().info("Destroyed swapchain");
    }

    void Swapchain::present()
    {
        const auto sync_interval = m_vsync_enabled ? 1u : 0u;
        const auto present_flags = m_tearing_supported && !m_vsync_enabled ? DXGI_PRESENT_ALLOW_TEARING : 0u;

        throw_if_failed(m_swapchain->Present(sync_interval, present_flags));

        m_current_backbuffer_index = m_swapchain->GetCurrentBackBufferIndex();
    }

    void Swapchain::check_for_tearing_support(const comptr<IDXGIFactory5> &factory)
    {
        auto allow_tearing = FALSE;

        // For vsync to be disabled, tearing must be suported by the swapchain.
        if (FAILED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing,
                                                sizeof(allow_tearing))))
        {
            m_tearing_supported = false;
        }

        m_tearing_supported = allow_tearing;
    }

    void Swapchain::update_rtvs(const comptr<ID3D12Device> &device, const DescriptorHeap &rtv_descriptor_heap,
                                const Uint2 dimension)
    {
        // The first NUM_BUFFER descriptors in the rtv_descriptor heap are reserved for swapchain back buffers.
        auto rtv_handle = rtv_descriptor_heap.get_handle_for_heap_start();

        auto back_buffer_index = 0u;
        for (auto &back_buffer : m_backbuffers)
        {
            throw_if_failed(m_swapchain->GetBuffer(back_buffer_index, IID_PPV_ARGS(&back_buffer.resource)));
            device->CreateRenderTargetView(back_buffer.resource.Get(), nullptr, rtv_handle.cpu_descriptor_handle);
            back_buffer.descriptor_handle = rtv_handle;

            rtv_handle.offset();

            ++back_buffer_index;
        }

        m_current_backbuffer_index = m_swapchain->GetCurrentBackBufferIndex();
    }

} // namespace serenity::graphics