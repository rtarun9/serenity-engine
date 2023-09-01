#pragma once

#include "serenity-engine/graphics/command_list.hpp"

namespace serenity::graphics
{
    CommandList::CommandList(const comptr<ID3D12Device> &const device, const D3D12_COMMAND_LIST_TYPE command_list_type)
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

    void CommandList::reset()
    {
        throw_if_failed(m_command_allocator->Reset());
        throw_if_failed(m_command_list->Reset(m_command_allocator.Get(), nullptr));
    }
} // namespace serenity::graphics