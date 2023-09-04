#pragma once

#include "serenity-engine/graphics/command_list.hpp"

#include "serenity-engine/graphics/device.hpp"

namespace serenity::graphics
{
    CommandList::CommandList(const comptr<ID3D12Device> &device, const D3D12_COMMAND_LIST_TYPE command_list_type)
        : m_command_list_type(command_list_type)
    {
        // Create command list and allocator.
        throw_if_failed(device->CreateCommandAllocator(m_command_list_type, IID_PPV_ARGS(&m_command_allocator)));
        throw_if_failed(device->CreateCommandList(0u, m_command_list_type, m_command_allocator.Get(), nullptr,
                                                  IID_PPV_ARGS(&m_command_list)));

        throw_if_failed(m_command_list->Close());

        // Set name for objects based on command queue / list type.
        const auto command_list_type_wstr = command_list_type_to_wstring(m_command_list_type);

        set_name(m_command_list.Get(), command_list_type_wstr + L" Command List");
        set_name(m_command_allocator.Get(), command_list_type_wstr + L" Command Allocator");

        core::Log::instance().info(
            std::format("Created command list and allocator of type {}", wstring_to_string(command_list_type_wstr)));
    }

    CommandList::~CommandList()
    {
        core::Log::instance().info(std::format("Destroyed command list and allocator of type {}",
                                               command_list_type_to_string(m_command_list_type)));
    }

    void CommandList::add_resource_barrier(const comptr<ID3D12Resource> &resource,
                                           const D3D12_RESOURCE_STATES previous_state,
                                           const D3D12_RESOURCE_STATES new_state)
    {
        m_resource_barriers.emplace_back(
            CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), previous_state, new_state));
    }

    void CommandList::execute_barriers()
    {
        m_command_list->ResourceBarrier(static_cast<uint32_t>(m_resource_barriers.size()), m_resource_barriers.data());
        m_resource_barriers.clear();
    }

    void CommandList::set_descriptor_heaps(const std::span<DescriptorHeap *const> descriptor_heaps)
    {
        auto shader_visible_descriptor_heaps = std::vector<ID3D12DescriptorHeap *>{};
        for (const auto &heaps : descriptor_heaps)
        {
            shader_visible_descriptor_heaps.emplace_back(heaps->get_descriptor_heap().Get());
        }

        m_command_list->SetDescriptorHeaps(static_cast<uint32_t>(descriptor_heaps.size()),
                                           shader_visible_descriptor_heaps.data());
    }

    void CommandList::reset()
    {
        throw_if_failed(m_command_allocator->Reset());
        throw_if_failed(m_command_list->Reset(m_command_allocator.Get(), nullptr));
    }

    void CommandList::clear_render_target_views(const DescriptorHandle rtv_descriptor_handle,
                                                const std::span<const float, 4u> clear_color) const
    {
        m_command_list->ClearRenderTargetView(rtv_descriptor_handle.cpu_descriptor_handle, clear_color.data(), 0u,
                                              nullptr);
    }

    void CommandList::clear_depth_stencil_view(const DescriptorHandle dsv_descriptor_handle, const float depth,
                                               const uint32_t stencil) const
    {
        m_command_list->ClearDepthStencilView(dsv_descriptor_handle.cpu_descriptor_handle, D3D12_CLEAR_FLAG_DEPTH,
                                              depth, stencil, 0u, nullptr);
    }

    void CommandList::set_render_targets(const std::span<const DescriptorHandle> rtv_descriptor_handle,
                                         const std::optional<DescriptorHandle> dsv_descriptor_handle) const
    {
        auto rtv_cpu_descriptor_handles = std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>{};
        for (const auto &handle : rtv_descriptor_handle)
        {
            rtv_cpu_descriptor_handles.emplace_back(handle.cpu_descriptor_handle);
        }

        if (dsv_descriptor_handle.has_value())
        {
            m_command_list->OMSetRenderTargets(static_cast<uint32_t>(rtv_cpu_descriptor_handles.size()),
                                               rtv_cpu_descriptor_handles.data(), true,
                                               &dsv_descriptor_handle->cpu_descriptor_handle);
        }
        else
        {
            m_command_list->OMSetRenderTargets(static_cast<uint32_t>(rtv_cpu_descriptor_handles.size()),
                                               rtv_cpu_descriptor_handles.data(), true, nullptr);
        }
    }

    void CommandList::set_primitive_topology(const D3D12_PRIMITIVE_TOPOLOGY primitive_topology) const
    {
        m_command_list->IASetPrimitiveTopology(primitive_topology);
    }

    void CommandList::set_index_buffer(const Buffer &buffer) const
    {
        const auto index_buffer_view = D3D12_INDEX_BUFFER_VIEW{
            .BufferLocation = buffer.resource.Get()->GetGPUVirtualAddress(),
            .SizeInBytes = static_cast<uint32_t>(buffer.size_in_bytes),
            .Format = DXGI_FORMAT_R16_UINT,
        };

        m_command_list->IASetIndexBuffer(&index_buffer_view);
    }

    void CommandList::set_bindless_graphics_root_signature() const
    {
        m_command_list->SetGraphicsRootSignature(RootSignature::instance().get_root_signature().Get());
    }

    void CommandList::set_pipeline_state(const Pipeline &pipeline) const
    {
        m_command_list->SetPipelineState(pipeline.get_pipeline_state().Get());
    }

    void CommandList::set_graphics_32_bit_root_constants(const std::byte *data) const
    {
        m_command_list->SetGraphicsRoot32BitConstants(0u, RootSignature::NUM_32_BIT_ROOT_CONSTANTS, data, 0u);
    }

    void CommandList::set_viewport_and_scissor_rect(const D3D12_VIEWPORT &viewport,
                                                    const D3D12_RECT &scissor_rect) const
    {
        m_command_list->RSSetViewports(1u, &viewport);
        m_command_list->RSSetScissorRects(1u, &scissor_rect);
    }

    void CommandList::draw_instanced(const uint32_t vertex_count, const uint32_t instance_count) const
    {
        m_command_list->DrawInstanced(vertex_count, instance_count, 0u, 0u);
    }

    void CommandList::draw_indexed_instanced(const uint32_t indices_count, const uint32_t instance_count) const
    {
        m_command_list->DrawIndexedInstanced(indices_count, instance_count, 0u, 0u, 0u);
    }
} // namespace serenity::graphics