#pragma once

#include "shaders/interop/constant_buffers.hlsli"

namespace serenity::renderer::renderpass
{
    // Implementation of A.J.Preetham's analytical model for daylight
    // (https://courses.cs.duke.edu/fall01/cps124/resources/p91-preetham.pdf).
    // Some notations to keep in mind : theta_s = angle between zenith and the sun.
    // theta - angle between view direction and zenith.
    // gamma - angle between sun and view direction.
    class AtmosphereRenderpass
    {
      public:
        explicit AtmosphereRenderpass();
        ~AtmosphereRenderpass();

        AtmosphereRenderPassBuffer &get_atmosphere_renderpass_buffer_data()
        {
            return m_atmosphere_buffer_data;
        }

        void update(const float sun_angle);

        uint32_t get_atmosphere_buffer_index() const
        {
            return m_atmosphere_buffer_index;
        }

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
        void compute_zenith_luminance(const float sun_angle);

      private:
        // The float3's A, B, C, D, E (within AtmosphereRenderpassBuffer) are part of the parameters required by the Perez et.al model to compute sky
        // luminance distribution. The other parameters are theta and gamma.
        // A corresponds to darkening / brightening of horizon.
        // B corresponds to luminance gradient near the horizon.
        // C corresponds to relative intensity of circumsolar region.
        // D corresponds to width of the circumsolar region.
        // E corresponds to relative backscattered light.
        // These terms can be computed with the help of turbidity value (as mentioned in the A.J Preetham paper section
        // A.2)
        // the X, Y, Z component of the float3 is for the Y luminance, x chromaticity, and y chromaticity.
        AtmosphereRenderPassBuffer m_atmosphere_buffer_data{};
        uint32_t m_atmosphere_buffer_index{};
    };
} // namespace serenity::renderer::renderpass