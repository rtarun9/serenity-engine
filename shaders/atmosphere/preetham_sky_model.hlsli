// clang-format off

#ifndef __PREETHAM_SKY_MODEL_HLSLI__
#define __PREETHAM_SKY_MODEL_HLSLI__

#include "interop/constant_buffers.hlsli"

// Some notations to keep in mind : 
// theta_s = angle between zenith and the sun.
// theta - angle between view direction and zenith.
// gamma - angle between sun and view direction.

// Formula (3) as mentioned in Preetham sky model paper.
float3 perez(const interop::PerezParameters perez_params, const float theta, const float gamma)
{
    return (1.0f + perez_params.A * exp(perez_params.B / max(cos(theta), 0.01f))) * (1.0f + perez_params.C * exp(perez_params.D * gamma) + perez_params.E * cos(gamma) * cos(gamma));
}

float3 preetham_sky_luminance_and_chromaticity(const interop::AtmosphereRenderPassBuffer atmosphere_buffer, const float3 view_direction, const float3 sun_direction)
{
    const float cos_gamma = dot(view_direction, sun_direction);
    const float gamma = acos(cos_gamma);

    // Since zenith is essentially the y axis, we can easily compute theta and theta_s by taking the dot product of view_direction and sun_direction
    // by float3(0.0f, 1.0f, 0.0f).

    const float theta = acos(saturate(dot(float3(0.0f, 1.0f, 0.0f), view_direction)));
    const float theta_s = acos(saturate(dot(float3(0.0f, 1.0f, 0.0f), sun_direction)));

    const float3 perez_theta_gamma = perez(atmosphere_buffer.perez_parameters, theta, gamma);
    const float3 perez_zero_theta_s = perez(atmosphere_buffer.perez_parameters, 0.0f, theta_s);
    
    const float3 perez_fraction = perez_theta_gamma / perez_zero_theta_s;

    const float3 result_yxy = atmosphere_buffer.zenith_luminance_chromaticity.xyz * perez_fraction;
    
    // Convertion from yxy to XYZ (based on formulas from https://en.wikipedia.org/wiki/CIE_1931_color_space).
    const float Y = result_yxy.r;
    const float x = result_yxy.g;
    const float y = result_yxy.b;

    const float3 result_xyz = float3(x * (Y / y), Y, (1.0f - x - y) * (Y / y));
    
    // Conversion from XYZ to RGB (https://colorcalculations.wordpress.com/rgb-color-spaces/#RGBspaces).
    const float r = dot(float3(2.370674, -0.900041, -0.470634), result_xyz);
    const float g = dot(float3(-0.513885, 1.425304, 0.088581), result_xyz);
    const float b = dot(float3(0.005298, -0.014695, 1.009397), result_xyz);

    return float3(r, g, b) * atmosphere_buffer.magnitude_multiplier;
}   

#endif 