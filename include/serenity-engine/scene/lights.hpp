#pragma once

#include "shaders/interop/constant_buffers.hlsli"

#include "serenity-engine/renderer/rhi/command_list.hpp"

namespace serenity::scene
{
    // Abstraction that holds all lights in the scene.
    class Lights
    {
      public:
        explicit Lights();

        void add_light(const Light &light);

        uint32_t get_light_buffer_index() const
        {
            return m_light_buffer_index;
        }

        LightBuffer &get_light_buffer()
        {
            return m_light_buffer;
        }

        void update(const math::XMMATRIX view_matrix);

        void render(const renderer::rhi::CommandList &command_list, const uint32_t scene_buffer_cbv_index);

      private:
        uint32_t m_light_buffer_index{};
        LightBuffer m_light_buffer{};

        // For visualization purposes.
        uint32_t m_cube_position_buffer_index{};
        uint32_t m_cube_index_buffer_index{};

        uint32_t m_light_pipeline_index{};
    };
} // namespace serenity::scene