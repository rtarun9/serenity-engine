#include "serenity-engine/renderer/renderpass/atmosphere_renderpass.hpp"

#include "serenity-engine/renderer/renderer.hpp"

namespace serenity::renderer::renderpass
{
    AtmosphereRenderpass::AtmosphereRenderpass()
    {
        core::Log::instance().info("Created atmosphere render pass");

        // Create buffers.
        m_atmosphere_buffer_index =
            Renderer::instance().create_buffer<AtmosphereRenderPassBuffer>(rhi::BufferCreationDesc{
                .usage = rhi::BufferUsage::ConstantBuffer,
                .name = L"Atmosphere Render Pass buffer",
            });

        m_atmosphere_buffer_data.turbidity = 2.0f;
    }

    AtmosphereRenderpass::~AtmosphereRenderpass()
    {
        core::Log::instance().info("Destroyed atmosphere render pass");
    }

    void AtmosphereRenderpass::update(const float sun_angle)
    {
        compute_perez_parameters();

        const auto sun_direction = math::XMFLOAT3(0.0f, -1.0f * sinf(sun_angle), -1.0f * cosf(sun_angle));

        // Theta_s is angle between sun and zenith, but while computed sun_direction, angle is between horizontal axis.
        const auto theta_s = acosf(sun_direction.y);

        compute_zenith_luminance(theta_s);

        Renderer::instance()
            .get_buffer_at_index(m_atmosphere_buffer_index)
            .update(reinterpret_cast<const std::byte *>(&m_atmosphere_buffer_data), sizeof(AtmosphereRenderPassBuffer));
    }

    void AtmosphereRenderpass::compute_perez_parameters()
    {
        // Formulas given in section A.2 of the A.J. Preetham paper.
        const auto turbidity = m_atmosphere_buffer_data.turbidity;

        // Calculation of luminance (Y), chromaticity (x) and chromaticity (y).
        m_atmosphere_buffer_data.perez_parameters.A = math::XMFLOAT3(
            0.1787f * turbidity - 1.4630f, -0.0193f * turbidity - 0.2592f, -0.0167f * turbidity - 0.2608f);

        m_atmosphere_buffer_data.perez_parameters.B = math::XMFLOAT3(
            -0.3554f * turbidity + 0.4275f, -0.0665f * turbidity + 0.0008f, -0.0950f * turbidity + 0.0092f);

        m_atmosphere_buffer_data.perez_parameters.C = math::XMFLOAT3(
            -0.0227f * turbidity + 5.3251f, -0.0004f * turbidity + 0.2125f, -0.0079f * turbidity + 0.2102f);

        m_atmosphere_buffer_data.perez_parameters.D = math::XMFLOAT3(
            0.1206f * turbidity - 2.5771f, -0.0641f * turbidity - 0.8989f, -0.0441f * turbidity - 1.6537f);

        m_atmosphere_buffer_data.perez_parameters.E = math::XMFLOAT3(
            -0.0670f * turbidity + 0.3703f, -0.0033f * turbidity + 0.0452f, -0.0109f * turbidity + 0.0529);
    }

    void AtmosphereRenderpass::compute_zenith_luminance(const float sun_angle)
    {
        const auto turbidity = m_atmosphere_buffer_data.turbidity;

        const auto extinction_factor = (4.0f / 9.0f - turbidity / 120.0f) * (math::XM_PI - 2.0f * sun_angle);

        m_atmosphere_buffer_data.zenith_luminance_chromaticity.x = 
            (4.0453f * turbidity - 4.9710f) * tanf(extinction_factor) - 0.2155f * turbidity + 2.4192f;

        const auto theta_s_vector = math::XMFLOAT4(pow(sun_angle, 3), pow(sun_angle, 2), pow(sun_angle, 1), 1);

        const auto turbidity_vector = math::XMFLOAT3(pow(turbidity, 2), turbidity, 1);

        m_atmosphere_buffer_data.zenith_luminance_chromaticity.y =
            (turbidity_vector.x * (0.0017f * theta_s_vector.x - 0.0037f * theta_s_vector.y +
                                   0.0021f * theta_s_vector.z + 0.0f * theta_s_vector.w)) +
            (turbidity_vector.y * (-0.0290f * theta_s_vector.x + 0.0638f * theta_s_vector.y -
                                   0.0320f * theta_s_vector.z + 0.0039f * theta_s_vector.w)) +
            (turbidity_vector.z * (0.1169f * theta_s_vector.x - 0.2120f * theta_s_vector.y +
                                   0.0605f * theta_s_vector.z + 0.2589f * theta_s_vector.w));

        m_atmosphere_buffer_data.zenith_luminance_chromaticity.z =
            (turbidity_vector.x * (0.0028f * theta_s_vector.x - 0.0061f * theta_s_vector.y +
                                   0.0032f * theta_s_vector.z + 0.0f * theta_s_vector.w)) +
            (turbidity_vector.y * (-0.0421 * theta_s_vector.x + 0.0897f * theta_s_vector.y -
                                   0.0415f * theta_s_vector.z + 0.0052f * theta_s_vector.w)) +
            (turbidity_vector.z * (0.1535f * theta_s_vector.x - 0.2676f * theta_s_vector.y +
                                   0.0667f * theta_s_vector.z + 0.2669f * theta_s_vector.w));
    }
} // namespace serenity::renderer::renderpass