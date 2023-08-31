#include "serenity-engine/graphics/device.hpp"

#include "serenity-engine/graphics/d3d_utils.hpp"

namespace serenity::graphics
{
    Device::Device(const HWND window_handle, const uint32_t width, const uint32_t height)
    {
        // Create the dxgi factory.
        auto factory_creation_flags = uint32_t{0};

        // Enable the debug layer in debug builds.
        if constexpr (SERENITY_DEBUG)
        {
            factory_creation_flags = DXGI_CREATE_FACTORY_DEBUG;

            throw_if_failed(::D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug_layer)));

            m_debug_layer->EnableDebugLayer();
            m_debug_layer->SetEnableAutoName(true);
            m_debug_layer->SetEnableGPUBasedValidation(true);
            m_debug_layer->SetEnableSynchronizedCommandQueueValidation(true);
        }

        throw_if_failed(::CreateDXGIFactory2(factory_creation_flags, IID_PPV_ARGS(&m_factory)));

        // Get the adapter with highest performance.
        throw_if_failed(
            m_factory->EnumAdapterByGpuPreference(0u, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_adapter)));

        throw_if_failed(::D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));
        set_name(m_device.Get(), L"D3D12 Device");

        // Create command queues.
        m_direct_command_queue = std::make_unique<CommandQueue>(m_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);

        // Create command list.
        m_direct_command_list = std::make_unique<CommandList>(m_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);

        // Create descriptor heaps.
        m_rtv_descriptor_heap = std::make_unique<DescriptorHeap>(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3u);

        // Create the swapchain.
        m_swapchain = std::make_unique<Swapchain>(m_factory.Get(), m_direct_command_queue->get_command_queue(), width,
                                                  height, window_handle);

        core::Log::get().info("Created graphics device");
    }

    Device::~Device()
    {
        core::Log::get().info("Destroyed graphics device");
    }
} // namespace serenity::graphics