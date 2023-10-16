#pragma once

#include "serenity-engine/renderer/rhi/command_list.hpp"
#include "serenity-engine/renderer/rhi/pipeline.hpp"

#include "serenity-engine/renderer/rhi/descriptor_heap.hpp"

#include "shaders/interop/constant_buffers.hlsli"

namespace serenity::renderer::renderpass
{
    // Implementation of A.J.Preetham's analytical model for daylight.
    // (https://courses.cs.duke.edu/fall01/cps124/resources/p91-preetham.pdf).
    class AtmosphereRenderpass
    {
      public:
        explicit AtmosphereRenderpass();
        ~AtmosphereRenderpass();

        interop::AtmosphereRenderPassBuffer &get_atmosphere_renderpass_buffer_data()
        {
            return m_atmosphere_buffer_data;
        }

        uint32_t get_atmosphere_buffer_index() const
        {
            return m_atmosphere_buffer_index;
        }

        uint32_t get_atmosphere_texture_index() const
        {
            return m_atmosphere_texture_index;
        }

        void update(const math::XMFLOAT3 sun_direction);
        void compute(rhi::CommandList &command_list, const uint32_t scene_buffer_cbv_index,
                     const uint32_t light_buffer_cbv_index) const;

      private:
        AtmosphereRenderpass(const AtmosphereRenderpass &other) = delete;
        AtmosphereRenderpass &operator=(const AtmosphereRenderpass &other) = delete;

        AtmosphereRenderpass(AtmosphereRenderpass &&other) = delete;
        AtmosphereRenderpass &operator=(AtmosphereRenderpass &&other) = delete;

        // Use the turbidity value to construct values of perez sky luminance model parameters (as described in section
        // A.2 in Preetham's paper).
        void compute_perez_parameters();

        // Compute the zenith luminance (and the extinction factor) based on turbidity and sun to zenith angle.
        // Formula mentioned in A.2 in Preetham's paper.
        void compute_zenith_luminance(const math::XMFLOAT3 sun_direction);

      public:
        static constexpr uint32_t ATMOSPHERE_TEXTURE_DIMENSION = 128u;

      private:
        // The float3's A, B, C, D, E (within AtmosphereRenderpassBuffer) are part of the parameters required by the
        // Perez et.al model to compute sky luminance distribution. The other parameters are theta and gamma. A
        // corresponds to darkening / brightening of horizon. B corresponds to luminance gradient near the horizon. C
        // corresponds to relative intensity of circumsolar region. D corresponds to width of the circumsolar region. E
        // corresponds to relative backscattered light. These terms can be computed with the help of turbidity value (as
        // mentioned in the A.J Preetham paper section A.2) the X, Y, Z component of the float3 is for the Y luminance,
        // x chromaticity, and y chromaticity.
        interop::AtmosphereRenderPassBuffer m_atmosphere_buffer_data{};
        uint32_t m_atmosphere_buffer_index{};

        uint32_t m_atmosphere_texture_index{};

        uint32_t m_preetham_sky_generation_pipeline_index{};
    };
} // namespace serenity::renderer::renderpass