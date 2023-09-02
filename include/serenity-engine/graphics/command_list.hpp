#pragma once

#include "d3d_utils.hpp"

#include "descriptor_heap.hpp"

namespace serenity::graphics
{
    // Command list's are used to record GPU commands that are submitted to a command queue for execution on the gpu.
    // Command allocator acts as a backing store for these recorded commands.
    // Resource barriers are expensive, hence this class allows batching of barriers.
    class CommandList
    {
      public:
        explicit CommandList(const comptr<ID3D12Device> &device, const D3D12_COMMAND_LIST_TYPE command_list_type);
        ~CommandList();

        comptr<ID3D12GraphicsCommandList> &get_command_list()
        {
            return m_command_list;
        }

        void add_resource_barrier(const comptr<ID3D12Resource> &resource, const D3D12_RESOURCE_STATES previous_state,
                                  const D3D12_RESOURCE_STATES new_state);

        // Executes resource barriers (batched).
        void execute_barriers();

        // Set shader visible descriptor heaps.
        void set_descriptor_heaps(const std::span<DescriptorHeap* const> descriptor_heaps);

        // Reset the command list and allocator.
        void reset();

      private:
        CommandList(const CommandList &other) = delete;
        CommandList &operator=(const CommandList &other) = delete;

        CommandList(CommandList &&other) = delete;
        CommandList &operator=(CommandList &&other) = delete;

      private:
        comptr<ID3D12GraphicsCommandList> m_command_list{};
        comptr<ID3D12CommandAllocator> m_command_allocator{};

        D3D12_COMMAND_LIST_TYPE m_command_list_type{};

        std::vector<D3D12_RESOURCE_BARRIER> m_resource_barriers{};
    };
} // namespace serenity::graphics