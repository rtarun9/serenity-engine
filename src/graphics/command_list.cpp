#pragma once

#include "serenity-engine/graphics/command_list.hpp"

namespace serenity::graphics
{
    CommandList::CommandList(ID3D12Device *const device, const D3D12_COMMAND_LIST_TYPE command_list_type)
        : m_command_list_type(command_list_type)
    {
        // Create command list and allocator.
        throw_if_failed(device->CreateCommandAllocator(m_command_list_type, IID_PPV_ARGS(&m_command_allocator)));
        throw_if_failed(device->CreateCommandList(0u, m_command_list_type, m_command_allocator.Get(), nullptr,
                                                  IID_PPV_ARGS(&m_command_list)));

        // Set name for objects based on command queue / list type.
        const auto command_list_type_str = command_list_type_to_string(m_command_list_type);

        set_name(m_command_list.Get(), command_list_type_str + L" Command List");
        set_name(m_command_allocator.Get(), command_list_type_str + L" Command Allocator");

        core::Log::get().info(
            std::format("Created command list and allocator of type {}", wstring_to_string(command_list_type_str)));
    }

    CommandList::~CommandList()
    {
        core::Log::get().info(std::format("Destroyed command list and allocator of type {}",
                                          wstring_to_string(command_list_type_to_string(m_command_list_type))));
    }
} // namespace serenity::graphics