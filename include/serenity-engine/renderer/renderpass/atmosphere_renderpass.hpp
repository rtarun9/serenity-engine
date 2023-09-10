#pragma once

#include "shaders/interop/constant_buffers.hlsli"

namespace serenity::renderer::renderpass
{
    // Implementation of A.J.Preetham's analytical model for daylight
    // (https://courses.cs.duke.edu/fall01/cps124/resources/p91-preetham.pdf).
    class AtmosphereRenderpass
    {
      public:
        explicit AtmosphereRenderpass();
        ~AtmosphereRenderpass();

        AtmosphereRenderPassBuffer &get_atmosphere_renderpass_buffer_data()
        {
            return m_atmosphere_buffer_data;
        }

        void update();

        uint32_t get_atmosphere_buffer_index() const
        {
            return m_atmosphere_buffer_index;
        }

      private:
        AtmosphereRenderpass(const AtmosphereRenderpass &other) = delete;
        AtmosphereRenderpass &operator=(const AtmosphereRenderpass &other) = delete;

        AtmosphereRenderpass(AtmosphereRenderpass &&other) = delete;
        AtmosphereRenderpass &operator=(AtmosphereRenderpass &&other) = delete;

      private:
        AtmosphereRenderPassBuffer m_atmosphere_buffer_data{};
        uint32_t m_atmosphere_buffer_index{};
    };
} // namespace serenity::renderer::renderpass