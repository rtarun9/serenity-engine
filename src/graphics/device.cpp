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

        m_dsv_descriptor_heap = std::make_unique<DescriptorHeap>(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 3u);
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

    Texture Device::create_texture(const TextureCreationDesc &texture_creation_desc, const std::byte *data)
    {
        auto texture = Texture{};

        // If the texture's data is not nullptr, then a upload buffer must be created to upload the data from cpu -> cpu
        // / gpu accesible memory, and finally copied into gpu only memory.
        if (!(texture_creation_desc.usage == TextureUsage::Depth ||
              texture_creation_desc.usage == TextureUsage::DepthStencil))
        {
            core::Log::instance().critical("This function has not been implemented yet!");
        }

        const auto texture_resource_desc = D3D12_RESOURCE_DESC{
            .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            .Alignment = 0u,
            .Width = static_cast<UINT64>(texture_creation_desc.dimension.x),
            .Height = texture_creation_desc.dimension.y,
            .DepthOrArraySize = 1u,
            .MipLevels = 1u,
            .Format = texture_creation_desc.format,
            .SampleDesc = {1u, 0u},
            .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
            .Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
        };

        const auto default_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        const auto clear_color = D3D12_CLEAR_VALUE{
            .Format = texture_creation_desc.format,
            .DepthStencil = {.Depth = 1.0f, .Stencil = 1u},
        };

        throw_if_failed(m_device->CreateCommittedResource(&default_heap_properties, D3D12_HEAP_FLAG_NONE,
                                                          &texture_resource_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                          &clear_color, IID_PPV_ARGS(&texture.resource)));

        // Create the depth stencil view.
        auto current_dsv_descriptor = m_dsv_descriptor_heap->get_current_handle();
        const auto dsv_desc = D3D12_DEPTH_STENCIL_VIEW_DESC{
            .Format = texture_creation_desc.format,
            .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
            .Texture2D =
                {
                    .MipSlice = 0u,
                },
        };

        m_device->CreateDepthStencilView(texture.resource.Get(), &dsv_desc,
                                         current_dsv_descriptor.cpu_descriptor_handle);
        texture.dsv_index = current_dsv_descriptor.index;

        m_dsv_descriptor_heap->offset_current_handle();

        return texture;
    }

    Pipeline Device::create_pipeline(const PipelineCreationDesc &pipeline_creation_desc)
    {
        return std::move(Pipeline(m_device, pipeline_creation_desc));
    }
} // namespace serenity::graphics