#include "serenity-engine/renderer/rhi/device.hpp"

#include "serenity-engine/renderer/rhi/d3d_utils.hpp"

// Setting up the agility SDK parameters.
extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 711u;
}

extern "C"
{
    __declspec(dllexport) extern const char *D3D12SDKPath = ".\\D3D12\\";
}

namespace serenity::renderer::rhi
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
            std::make_unique<DescriptorHeap>(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 10'000u);

        m_dsv_descriptor_heap = std::make_unique<DescriptorHeap>(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 3u);

        // Create the swapchain.
        m_swapchain = std::make_unique<Swapchain>(m_factory, m_device, m_direct_command_queue->get_command_queue(),
                                                  *(m_rtv_descriptor_heap.get()), dimension, window_handle);

        // Create the root signature.
        m_root_signature = std::make_unique<RootSignature>(m_device);

        core::Log::instance().info("Created graphics device");
    }

    Device::~Device()
    {
        core::Log::instance().info("Destroyed graphics device");
    }

    void Device::frame_start()
    {
        // Reset command buffer and allocater associated with this frame.
        m_current_swapchain_backbuffer_index = m_swapchain->get_current_backbuffer_index();

        m_direct_command_lists.at(m_current_swapchain_backbuffer_index)->reset();
    }

    void Device::frame_end()
    {
        m_frame_fence_values.at(m_current_swapchain_backbuffer_index) = m_direct_command_queue->signal();
        m_current_swapchain_backbuffer_index = m_swapchain->get_current_backbuffer_index();

        // Wait for the previous frame (i.e the new m_current_swapchain_backbuffer_index's previous command's) to finish
        // execution.
        m_direct_command_queue->wait_for_fence_value(m_frame_fence_values.at(m_current_swapchain_backbuffer_index));
    }

    Texture Device::create_texture(const TextureCreationDesc &texture_creation_desc, const std::byte *data)
    {
        auto texture = Texture{};

        // If the texture's data is not nullptr, then a upload buffer must be created to upload the data from cpu -> cpu
        // / gpu accesible memory, and finally copied into gpu only memory. This willl be the case for most textures.

        if (!(texture_creation_desc.usage == TextureUsage::DepthStencilTexture ||
              texture_creation_desc.usage == TextureUsage::ShaderResourceTexture))
        {
            core::Log::instance().critical("This function has not been implemented yet!");
        }

        const auto get_texture_flags = [&]() {
            switch (texture_creation_desc.usage)
            {
            case TextureUsage::DepthStencilTexture: {
                return D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            }
            break;

            case TextureUsage::ShaderResourceTexture: {
                return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }
            break;

            case TextureUsage::UAVTexture: {
                return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }
            break;

            case TextureUsage::RenderTexture: {
                return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            }
            break;

            default: {
                return D3D12_RESOURCE_FLAG_NONE;
            }
            break;
            }
        };

        // Create resource (GPU only).
        const auto texture_resource_desc = D3D12_RESOURCE_DESC{
            .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            .Alignment = 0u,
            .Width = static_cast<UINT64>(texture_creation_desc.dimension.x),
            .Height = texture_creation_desc.dimension.y,
            .DepthOrArraySize = 1u,
            .MipLevels = static_cast<UINT16>(texture_creation_desc.mip_levels),
            .Format = texture_creation_desc.format,
            .SampleDesc = {1u, 0u},
            .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
            .Flags = get_texture_flags(),
        };

        const auto default_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        // Clear color is only valid for depth texture and render target's.
        if (texture_creation_desc.usage == TextureUsage::DepthStencilTexture)
        {
            const auto depth_clear_color = D3D12_CLEAR_VALUE{
                .Format = texture_creation_desc.format,
                .DepthStencil = {.Depth = 1.0f, .Stencil = 1u},
            };

            throw_if_failed(m_device->CreateCommittedResource(&default_heap_properties, D3D12_HEAP_FLAG_NONE,
                                                              &texture_resource_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                              &depth_clear_color, IID_PPV_ARGS(&texture.resource)));
        }
        else if (texture_creation_desc.usage == TextureUsage::RenderTexture)
        {
            const auto render_target_clear_color = D3D12_CLEAR_VALUE{
                .Format = texture_creation_desc.format,
                .Color = {0.0f, 0.0f, 0.0f, 1.0f},
            };

            throw_if_failed(
                m_device->CreateCommittedResource(&default_heap_properties, D3D12_HEAP_FLAG_NONE,
                                                  &texture_resource_desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                                  &render_target_clear_color, IID_PPV_ARGS(&texture.resource)));
        }
        else
        {
            throw_if_failed(m_device->CreateCommittedResource(&default_heap_properties, D3D12_HEAP_FLAG_NONE,
                                                              &texture_resource_desc, D3D12_RESOURCE_STATE_COPY_DEST,
                                                              nullptr, IID_PPV_ARGS(&texture.resource)));
        }

        // If data is to be filled with some initial data, create a CPU / GPU accessible heap - resource and upload the
        // data.
        if (data)
        {
            // Here, we need to create another texture so as to copy data from CPU / GPU shareable memory to GPU only
            // memory.

            const auto size = texture_creation_desc.dimension.x * texture_creation_desc.dimension.y *
                              texture_creation_desc.bytes_per_pixel;

            const auto upload_buffer_resource_desc = CD3DX12_RESOURCE_DESC::Buffer(size);

            const auto upload_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            auto upload_buffer = comptr<ID3D12Resource>{};

            throw_if_failed(m_device->CreateCommittedResource(&upload_heap_properties, D3D12_HEAP_FLAG_NONE,
                                                              &upload_buffer_resource_desc, D3D12_RESOURCE_STATE_COMMON,
                                                              nullptr, IID_PPV_ARGS(&upload_buffer)));

            // Copy data from CPU to GPU.
            const auto subresource_data = D3D12_SUBRESOURCE_DATA{
                .pData = data,
                .RowPitch =
                    static_cast<LONG_PTR>(texture_creation_desc.bytes_per_pixel * texture_creation_desc.dimension.x),
                .SlicePitch = size,
            };

            m_copy_command_list->reset();
            UpdateSubresources(m_copy_command_list->get_command_list().Get(), texture.resource.Get(),
                               upload_buffer.Get(), 0u, 0u, 1u, &subresource_data);

            const auto command_list_for_execution = std::array{
                m_copy_command_list.get(),
            };

            m_copy_command_queue->execute(command_list_for_execution);
            m_copy_command_queue->flush();
        }

        // Create descriptors based on texture usage.
        if (texture_creation_desc.usage == TextureUsage::DepthStencilTexture)
        {
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
        }

        if (texture_creation_desc.usage == TextureUsage::ShaderResourceTexture)
        {
            // Create the shader resource view.
            auto current_srv_descriptor = m_cbv_srv_uav_descriptor_heap->get_current_handle();

            const auto srv_desc = D3D12_SHADER_RESOURCE_VIEW_DESC{
                .Format = texture_creation_desc.format,
                .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
                .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
                .Texture2D{
                    .MostDetailedMip = 0u,
                    .MipLevels = texture_creation_desc.mip_levels,
                },
            };

            m_device->CreateShaderResourceView(texture.resource.Get(), &srv_desc,
                                               current_srv_descriptor.cpu_descriptor_handle);
            texture.srv_index = current_srv_descriptor.index;

            m_cbv_srv_uav_descriptor_heap->offset_current_handle();
        }

        return texture;
    }

    Pipeline Device::create_pipeline(const PipelineCreationDesc &pipeline_creation_desc)
    {
        return std::move(Pipeline(m_device, pipeline_creation_desc));
    }
} // namespace serenity::renderer::rhi