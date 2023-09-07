#include "serenity-engine/renderer/rhi/command_queue.hpp"

namespace serenity::renderer::rhi
{
    CommandQueue::CommandQueue(const comptr<ID3D12Device> &device, const D3D12_COMMAND_LIST_TYPE command_list_type)
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
        const auto command_list_type_wstr = command_list_type_to_wstring(m_command_list_type);

        set_name(m_command_queue.Get(), command_list_type_wstr + L" Command Queue");
        set_name(m_fence.Get(), command_list_type_wstr + L" Fence");

        core::Log::instance().info(
            std::format("Created command queue of type {}", wstring_to_string(command_list_type_wstr)));
    }

    CommandQueue::~CommandQueue()
    {
        core::Log::instance().info(
            std::format("Destroyed command queue of type {}", command_list_type_to_string(m_command_list_type)));
    }

    uint64_t CommandQueue::signal()
    {
        m_monotonically_increasing_fence_value++;

        throw_if_failed(m_command_queue->Signal(m_fence.Get(), m_monotonically_increasing_fence_value));
        return m_monotonically_increasing_fence_value;
    }

    void CommandQueue::wait_for_fence_value(const uint64_t fence_value)
    {
        if (m_fence->GetCompletedValue() < fence_value)
        {
            throw_if_failed(m_fence->SetEventOnCompletion(fence_value, nullptr));
        }
    }

    void CommandQueue::flush()
    {
        signal();
        wait_for_fence_value(m_monotonically_increasing_fence_value);
    }

    void CommandQueue::execute(const std::span<CommandList *const> command_lists)
    {
        // Close all command lists.
        auto command_lists_to_execute = std::vector<ID3D12CommandList *>{};

        for (auto &list : command_lists)
        {
            throw_if_failed(list->get_command_list()->Close());
            command_lists_to_execute.emplace_back(list->get_command_list().Get());
        }

        m_command_queue->ExecuteCommandLists(static_cast<uint32_t>(command_lists_to_execute.size()),
                                             command_lists_to_execute.data());
    }

} // namespace serenity::renderer::rhi