#pragma once

#include "d3d_utils.hpp"

#include "buffer.hpp"
#include "command_signature.hpp"
#include "descriptor_heap.hpp"
#include "pipeline.hpp"

namespace serenity::renderer::rhi
{
    // Command list's are used to record GPU commands that are submitted to a command queue for execution on the gpu.
    // Command allocator acts as a backing store for these recorded commands.
    // Resource barriers are expensive, hence this class allows batching of barriers.
    // NOTE : For now, the same class will contain methods for setting state of the compute and graphics pipeline. If a
    // invalid call is made, the class will not handle the error, but the D3D12 debug layers will.
    class CommandList
    {
      public:
        explicit CommandList(const comptr<ID3D12Device> &device, const D3D12_COMMAND_LIST_TYPE command_list_type);
        ~CommandList();

        comptr<ID3D12GraphicsCommandList> &get_command_list() { return m_command_list; }

        void add_resource_barrier(const comptr<ID3D12Resource> &resource, const D3D12_RESOURCE_STATES previous_state,
                                  const D3D12_RESOURCE_STATES new_state);

        // Executes resource barriers (batched).
        void execute_barriers();

        // Set shader visible descriptor heaps.
        void set_descriptor_heaps(const std::span<DescriptorHeap *const> descriptor_heaps);

        // Reset the command list and allocator.
        void reset();

        // If in future graphics and compute command list are created, these functions will have to be split up since
        // these are pipeline and (command list) type specific. Added here for now keeping in mind that d3d12 debug
        // layer will indicate of any invalid mix/matches in command list type and functions we are trying to call.
        // Example : calling IASetPrimitiveToplogy on a command list of type COPY should not be done and this class
        // won't prevent user from doing that, but debug layer will set breakpoint and tell about the error.

        // Graphics (Direct) command list functions.
        void clear_render_target_views(const DescriptorHandle rtv_descriptor_handle,
                                       const std::span<const float, 4u> clear_color) const;

        void clear_depth_stencil_view(const DescriptorHandle dsv_descriptor_handle, const float depth = 1.0f,
                                      const uint32_t stencil = 1u) const;

        void set_render_targets(const std::span<const DescriptorHandle> rtv_descriptor_handle,
                                const std::optional<DescriptorHandle> dsv_descriptor_handle = {}) const;

        void set_primitive_topology(const D3D12_PRIMITIVE_TOPOLOGY primitive_topology) const;

        void set_index_buffer(const Buffer &buffer) const;

        void set_bindless_graphics_root_signature() const;

        void set_pipeline_state(const Pipeline &pipeline) const;

        void set_graphics_32_bit_root_constants(const std::byte *data) const;

        void set_viewport_and_scissor_rect(const D3D12_VIEWPORT &viewport, const D3D12_RECT &scissor_rect) const;

        void draw_instanced(const uint32_t vertex_count, const uint32_t instance_count) const;
        void draw_indexed_instanced(const uint32_t indices_count, const uint32_t instance_count) const;

        // Compute command list functions.
        void set_bindless_compute_root_signature() const;

        void set_compute_32_bit_root_constants(const std::byte *data) const;

        void dispatch(const Uint3 num_groups) const;

        // Common functions.
        void execute_indirect(rhi::CommandSignature &command_signature, const Buffer &indirect_argument_buffer,
                              const uint32_t command_count) const;

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
} // namespace serenity::renderer::rhi