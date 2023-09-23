#pragma once

#include "shaders/interop/constant_buffers.hlsli"

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

        void update();

      private:
        uint32_t m_light_buffer_index{};
        LightBuffer m_light_buffer{};
    };
} // namespace serenity::scene