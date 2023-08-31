#include "serenity-engine/graphics/command_queue.hpp"

namespace serenity::graphics
{
    CommandQueue::CommandQueue(ID3D12Device *const device, const D3D12_COMMAND_LIST_TYPE command_list_type)
        : m_command_list_type(command_list_type)
    {
        // Create command queue.
        const auto command_queue_desc = D3D12_COMMAND_QUEUE_DESC{
            .Type = command_list_type,
            .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
            .NodeMask = 0u,
        };

        throw_if_failed(device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&m_command_queue)));

        // Create fence object for synchronization.
        throw_if_failed(device->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

        // Set name for objects based on command queue / list type.
        const auto command_list_type_str = command_list_type_to_string(m_command_list_type);

        set_name(m_command_queue.Get(), command_list_type_str + L" Command Queue");

        core::Log::get().info(
            std::format("Created command queue of type {}", wstring_to_string(command_list_type_str)));
    }

    CommandQueue::~CommandQueue()
    {
        core::Log::get().info(std::format("Destroyed command queue of type {}",
                                          wstring_to_string(command_list_type_to_string(m_command_list_type))));
    }

} // namespace serenity::graphics