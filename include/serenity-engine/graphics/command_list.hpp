#pragma once

#include "d3d_utils.hpp"

namespace serenity::graphics
{
    // Command list's are used to record GPU commands that are submitted to a command queue for execution on the gpu.
    // Command allocator acts as a backing store for these recorded commands.
    class CommandList
    {
      public:
        explicit CommandList(ID3D12Device *const device, const D3D12_COMMAND_LIST_TYPE command_list_type);
        ~CommandList();

      private:
        CommandList(const CommandList &other) = delete;
        CommandList &operator=(const CommandList &other) = delete;

        CommandList(CommandList &&other) = delete;
        CommandList &operator=(CommandList &&other) = delete;

      private:
        comptr<ID3D12GraphicsCommandList> m_command_list{};
        comptr<ID3D12CommandAllocator> m_command_allocator{};

        D3D12_COMMAND_LIST_TYPE m_command_list_type{};
    };
} // namespace serenity::graphics