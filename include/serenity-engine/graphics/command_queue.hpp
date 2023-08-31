#pragma once

#include "d3d_utils.hpp"

namespace serenity::graphics
{
    // Command Queue is the execution port of the GPU.
    // Command lists are submitted to the queue for execution on the GPU. Since synchronization must be done explicitly,
    // sync primitives are present within this class.
    class CommandQueue
    {
      public:
        explicit CommandQueue(ID3D12Device *const device, const D3D12_COMMAND_LIST_TYPE command_list_type);
        ~CommandQueue();

        ID3D12CommandQueue *get_command_queue() const
        {
            return m_command_queue.Get();
        }

      private:
        CommandQueue(const CommandQueue &other) = delete;
        CommandQueue &operator=(const CommandQueue &other) = delete;

        CommandQueue(CommandQueue &&other) = delete;
        CommandQueue &operator=(CommandQueue &&other) = delete;

      private:
        comptr<ID3D12CommandQueue> m_command_queue{};
        comptr<ID3D12Fence> m_fence{};

        D3D12_COMMAND_LIST_TYPE m_command_list_type{};
    };
} // namespace serenity::graphics