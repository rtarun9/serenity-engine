#pragma once

#include "command_list.hpp"
#include "command_queue.hpp"
#include "descriptor_heap.hpp"
#include "root_signature.hpp"
#include "swapchain.hpp"

#include "buffer.hpp"
#include "d3d_utils.hpp"
#include "pipeline.hpp"
#include "texture.hpp"

#include "serenity-engine/core/singleton_instance.hpp"

namespace serenity::renderer::rhi
{
    // Abstraction for creating / destroying various graphics resources.
    // Encapsulates most renderer resources / objects in use : the swap chain, descriptor heaps, command queue's, etc.
    class Device
    {
      public:
        explicit Device(const HWND window_handle, const Uint2 dimensions);
        ~Device();

        comptr<ID3D12Device> get_device() const
        {
            return m_device;
        }

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

        DescriptorHeap &get_cbv_srv_uav_descriptor_heap() const
        {
            return *(m_cbv_srv_uav_descriptor_heap.get());
        }

        DescriptorHeap &get_rtv_descriptor_heap() const
        {
            return *(m_rtv_descriptor_heap.get());
        }

        DescriptorHeap &get_dsv_descriptor_heap() const
        {
            return *(m_dsv_descriptor_heap.get());
        }

        Swapchain &get_swapchain() const
        {
            return *(m_swapchain.get());
        }

        // Resets the command buffer and allocator for the current frame. Also gets the swapchain backbuffer for this
        // frame.
        void frame_start();

        // Updates the frame fence value for current frame and updates current swapchain backbuffer index.
        // Handles synchronization for now as well.
        void frame_end();

        // Functions for creation of various resources.
        template <typename T>
        [[nodiscard]] Buffer create_buffer(const BufferCreationDesc &buffer_creation_desc,
                                           const std::span<const T> data = {});

        [[nodiscard]] Texture create_texture(const TextureCreationDesc &texture_creation_desc,
                                             const std::byte *data = nullptr);

        [[nodiscard]] Pipeline create_pipeline(const PipelineCreationDesc &pipeline_creation_desc);

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
        std::unique_ptr<CommandQueue> m_copy_command_queue{};

        // Sync primitives.
        std::array<uint64_t, FRAMES_IN_FLIGHT> m_frame_fence_values{};

        // Command list are used for recording gpu commands that are submitted for execution to the command queue.
        // Using a unique command list per frame in flight.
        std::array<std::unique_ptr<CommandList>, FRAMES_IN_FLIGHT> m_direct_command_lists{};
        std::unique_ptr<CommandList> m_copy_command_list{};

        // Descriptor heaps are contiguous memory allocations of descriptors (which in turn are small blocks of memory
        // that fully describe a resource to the gpu).
        std::unique_ptr<DescriptorHeap> m_rtv_descriptor_heap{};
        std::unique_ptr<DescriptorHeap> m_cbv_srv_uav_descriptor_heap{};
        std::unique_ptr<DescriptorHeap> m_dsv_descriptor_heap{};

        // Swapchain implements / holds surfaces that we can render onto / store for presentation.
        // handles swapping of buffers.
        // This current swapchain back buffer index is updated in the frame_start() and frame_end() functions.
        std::unique_ptr<Swapchain> m_swapchain{};
        uint32_t m_current_swapchain_backbuffer_index{};

        // Bindless root signature (a singleton instance).
        std::unique_ptr<RootSignature> m_root_signature{};
    };

    template <typename T>
    inline Buffer Device::create_buffer(const BufferCreationDesc &buffer_creation_desc, const std::span<const T> data)
    {
        if (buffer_creation_desc.usage != BufferUsage::ConstantBuffer && data.empty())
        {
            core::Log::instance().critical(
                std::format("Attempting to create buffer with name {} with no data (Only constant "
                            "buffers can be created with no initial data",
                            wstring_to_string(buffer_creation_desc.name)));
        }

        auto buffer = Buffer{};

        const auto size = sizeof(T) * std::max<size_t>(data.size(), static_cast<size_t>(1u));
        buffer.size_in_bytes = size;

        // Create a commited resource for the buffer (which creates a heap (i.e abstarction of contiguous memory on GPU)
        // large enough to contain entire resource, which is mapped to the heap.

        // This heap will be accessible by the CPU, so the heap type will be UPLOAD.
        const auto upload_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const auto resource_desc = CD3DX12_RESOURCE_DESC::Buffer(size);

        auto upload_buffer = comptr<ID3D12Resource>{};

        throw_if_failed(m_device->CreateCommittedResource(&upload_heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
                                                          D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                          IID_PPV_ARGS(&upload_buffer)));

        // Fill in for constant buffer.
        if (buffer_creation_desc.usage == BufferUsage::ConstantBuffer)
        {
            // For constant buffers, there is no GPU accesible only buffer. Due to this, the upload buffer is (despite
            // what the name might suggest) the resource we want.
            buffer.resource = upload_buffer;

            // Get a mapped pointer to this constant buffer so we can write to it from the CPU.
            auto mapped_pointer = static_cast<uint8_t *>(nullptr);

            const auto no_read_range = D3D12_RANGE{
                .Begin = 0u,
                .End = 0u,
            };

            throw_if_failed(buffer.resource->Map(0u, &no_read_range, reinterpret_cast<void **>(&mapped_pointer)));
            buffer.mapped_pointer = mapped_pointer;

            if (data.size() > 0)
            {
                // If some data is passed in, update the constant buffer.
                buffer.update(reinterpret_cast<const std::byte *>(data.data()), sizeof(T));
            }
        }
        else
        {
            // In this case, we need to create another resource that will be readable only by the GPU (for best
            // bandwidth), and copy data from upload buffer to here.

            const auto default_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            throw_if_failed(m_device->CreateCommittedResource(&default_heap_properties, D3D12_HEAP_FLAG_NONE,
                                                              &resource_desc, D3D12_RESOURCE_STATE_COMMON, nullptr,
                                                              IID_PPV_ARGS(&buffer.resource)));

            // Copy data from CPU to GPU.
            const auto subresource_data = D3D12_SUBRESOURCE_DATA{
                .pData = data.data(),
                .RowPitch = static_cast<LONG_PTR>(size),
                .SlicePitch = static_cast<LONG_PTR>(size),
            };

            m_copy_command_list->reset();
            UpdateSubresources(m_copy_command_list->get_command_list().Get(), buffer.resource.Get(),
                               upload_buffer.Get(), 0u, 0u, 1u, &subresource_data);

            const auto command_list_for_execution = std::array{
                m_copy_command_list.get(),
            };

            m_copy_command_queue->execute(command_list_for_execution);
            m_copy_command_queue->flush();
        }

        // Based on buffer usage, create the descriptors.
        switch (buffer_creation_desc.usage)
        {
        case BufferUsage::StructuredBuffer: {
            // Create SRV for structured buffer.
            const auto srv_desc = D3D12_SHADER_RESOURCE_VIEW_DESC{
                .Format = DXGI_FORMAT_UNKNOWN,
                .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
                .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
                .Buffer =
                    {
                        .FirstElement = 0u,
                        .NumElements = static_cast<uint32_t>(data.size()),
                        .StructureByteStride = sizeof(T),
                    },
            };

            const auto current_srv_descriptor = m_cbv_srv_uav_descriptor_heap->get_current_handle();
            m_device->CreateShaderResourceView(buffer.resource.Get(), &srv_desc,
                                               current_srv_descriptor.cpu_descriptor_handle);

            buffer.srv_index = current_srv_descriptor.index;
            m_cbv_srv_uav_descriptor_heap->offset_current_handle();
        }
        break;

        case BufferUsage::ConstantBuffer: {
            const auto cbv_desc = D3D12_CONSTANT_BUFFER_VIEW_DESC{
                .BufferLocation = buffer.resource.Get()->GetGPUVirtualAddress(),
                .SizeInBytes = static_cast<uint32_t>(buffer.size_in_bytes),
            };

            const auto current_cbv_descriptor = m_cbv_srv_uav_descriptor_heap->get_current_handle();
            m_device->CreateConstantBufferView(&cbv_desc, current_cbv_descriptor.cpu_descriptor_handle);

            buffer.cbv_index = current_cbv_descriptor.index;
            m_cbv_srv_uav_descriptor_heap->offset_current_handle();
        }
        break;
        };

        set_name(buffer.resource.Get(), buffer_creation_desc.name);

        core::Log::instance().info(std::format("Created {} with name {}",
                                               buffer_usage_to_string(buffer_creation_desc.usage),
                                               wstring_to_string(buffer_creation_desc.name)));

        return buffer;
    }
} // namespace serenity::renderer::rhi