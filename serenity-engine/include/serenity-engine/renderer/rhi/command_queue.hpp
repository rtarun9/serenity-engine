#pragma once

#include "d3d_utils.hpp"

#include "command_list.hpp"

namespace serenity::renderer::rhi
{
    // Command Queue is the execution port of the GPU.
    // Command lists are submitted to the queue for execution on the GPU. Since synchronization must be done explicitly,
    // sync primitives are present within this class.
    class CommandQueue
    {
      public:
        explicit CommandQueue(const comptr<ID3D12Device> &device, const D3D12_COMMAND_LIST_TYPE command_list_type);
        ~CommandQueue();

        comptr<ID3D12CommandQueue> &get_command_queue() { return m_command_queue; }

        // Return the fence value we need to wait on to ensure that the GPU has completed execution for instructions
        // (including this current signal call).
        uint64_t signal();

        // Block the cpu thread until the fence value has reached the given value (i.e fence->GetCompletedValue() must
        // be atleast equal to fence_value for this function to unblock the CPU thread).
        void wait_for_fence_value(const uint64_t fence_value);

        // Wait for GPU to complete execution of all instructions given to it.
        // Returns updated value of the monotonically increasing fence value.
        uint64_t flush();

        // Execute the command lists.
        void execute(const std::span<CommandList *const> command_lists);

      private:
        CommandQueue(const CommandQueue &other) = delete;
        CommandQueue &operator=(const CommandQueue &other) = delete;

        CommandQueue(CommandQueue &&other) = delete;
        CommandQueue &operator=(CommandQueue &&other) = delete;

      private:
        comptr<ID3D12CommandQueue> m_command_queue{};
        comptr<ID3D12Fence> m_fence{};

        D3D12_COMMAND_LIST_TYPE m_command_list_type{};

        uint64_t m_monotonically_increasing_fence_value{};
    };
} // namespace serenity::renderer::rhi