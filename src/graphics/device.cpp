#include "serenity-engine/graphics/device.hpp"

#include "serenity-engine/graphics/d3d_utils.hpp"

// Setting up the agility SDK parameters.
extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 711u;
}

extern "C"
{
    __declspec(dllexport) extern const char *D3D12SDKPath = ".\\D3D12\\";
}

namespace serenity::graphics
{
    Device::Device(const HWND window_handle, const Uint2 dimension)
    {
        // Create the dxgi factory to enumerate adapters, screen modes, etc.
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

        auto adapter_desc = DXGI_ADAPTER_DESC1{};
        throw_if_failed(m_adapter->GetDesc1(&adapter_desc));
        core::Log::instance().info(std::format("Selected adapter : {}", wstring_to_string(adapter_desc.Description)));

        throw_if_failed(::D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));
        set_name(m_device.Get(), L"D3D12 Device");

        // Setup info queue in debug builds to place break points on occurance of errors / warnings (based on severity).
        if constexpr (SERENITY_DEBUG)
        {
            comptr<ID3D12InfoQueue1> info_queue{};
            throw_if_failed(m_device.As(&info_queue));

            throw_if_failed(info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true));
            throw_if_failed(info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true));
            throw_if_failed(info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true));
        }

        // Create command queues.
        m_direct_command_queue = std::make_unique<CommandQueue>(m_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
        m_copy_command_queue = std::make_unique<CommandQueue>(m_device.Get(), D3D12_COMMAND_LIST_TYPE_COPY);

        // Create command lists.
        for (auto &command_list : m_direct_command_lists)
        {
            command_list = std::make_unique<CommandList>(m_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
        }

        m_copy_command_list = std::make_unique<CommandList>(m_device.Get(), D3D12_COMMAND_LIST_TYPE_COPY);

        // Create descriptor heaps.
        m_rtv_descriptor_heap = std::make_unique<DescriptorHeap>(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3u);
        m_rtv_descriptor_heap->offset_current_handle(Swapchain::NUM_BACK_BUFFERS);

        m_cbv_srv_uav_descriptor_heap =
            std::make_unique<DescriptorHeap>(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100u);

        // Create the swapchain.
        m_swapchain = std::make_unique<Swapchain>(m_factory, m_device, m_direct_command_queue->get_command_queue(),
                                                  *(m_rtv_descriptor_heap.get()), dimension, window_handle);

        // Create the root signature.
        m_root_signature = std::make_unique<RootSignature>(m_device);

        // Create the shader compiler.
        m_shader_compiler = std::make_unique<ShaderCompiler>();

        core::Log::instance().info("Created graphics device");
    }

    Device::~Device()
    {
        core::Log::instance().info("Destroyed graphics device");
    }

    void Device::frame_start()
    {
        // Reset command buffer and allocater associated with this frame.
        m_current_swapchain_backbuffer_index = Swapchain::instance().get_current_backbuffer_index();

        m_direct_command_lists.at(m_current_swapchain_backbuffer_index)->reset();
    }

    void Device::frame_end()
    {
        m_frame_fence_values.at(m_current_swapchain_backbuffer_index) = m_direct_command_queue->signal();
        m_current_swapchain_backbuffer_index = Swapchain::instance().get_current_backbuffer_index();

        // Wait for the previous frame (i.e the new m_current_swapchain_backbuffer_index's previous command's) to finish
        // execution.
        m_direct_command_queue->wait_for_fence_value(m_frame_fence_values.at(m_current_swapchain_backbuffer_index));
    }

    Pipeline Device::create_pipeline(const PipelineCreationDesc &pipeline_creation_desc)
    {
        return std::move(Pipeline(m_device, pipeline_creation_desc));
    }
} // namespace serenity::graphics