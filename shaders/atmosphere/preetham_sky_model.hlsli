// clang-format off

#ifndef __PREETHAM_SKY_MODEL_HLSLI__
#define __PREETHAM_SKY_MODEL_HLSLI__

#include "interop/constant_buffers.hlsli"

// Some notations to keep in mind : 
// theta_s = angle between zenith and the sun.
// theta - angle between view direction and zenith.
// gamma - angle between sun and view direction.

// Formula (3) as mentioned in Preetham sky model paper.
float3 perez(const PerezParameters perez_params, const float theta, const float gamma)
{
    return (1.0f + perez_params.A * exp(perez_params.B / cos(theta))) * (1.0f + perez_params.C * exp(perez_params.D * gamma) + perez_params.E * (cos(gamma) * cos(gamma)));
}

float3 preetham_sky_luminance_and_chromaticity(const AtmosphereRenderPassBuffer atmosphere_buffer, const float3 view_direction, const float3 sun_direction, const float magnitude_multiplier)
{
    const float cos_gamma = saturate(dot(view_direction, sun_direction));
    const float gamma = acos(cos_gamma);

    // Since zenith is essentially the y axis, we can easily compute theta and theta_s by taking the dot product of view_direction and sun_direction
    // by float3(0.0f, 1.0f, 0.0f).

    const float theta = acos(saturate(dot(float3(0.0f, 1.0f, 0.0f), view_direction)));
    const float theta_s = acos(saturate(dot(float3(0.0f, 1.0f, 0.0f), sun_direction)));

    const float3 perez_theta_gamma = perez(atmosphere_buffer.perez_parameters, theta, gamma);
    const float3 perez_zero_theta_s = perez(atmosphere_buffer.perez_parameters, 0.0f, theta_s);
    
    const float3 perez_fraction = perez_theta_gamma / perez_zero_theta_s;

    // Normalizing luminance in LDR cases (which is always for now), to have fixed value for sun.
    const float luminance = atmosphere_buffer.zenith_luminance_chromaticity.x / perez(atmosphere_buffer.perez_parameters, theta_s, 0.0f).x;

    const float3 result_yxy = float3(luminance, atmosphere_buffer.zenith_luminance_chromaticity.yz) * perez_fraction;
    
    // Convertion from yxy to XYZ (based on formulas from https://en.wikipedia.org/wiki/CIE_1931_color_space).
    const float Y = result_yxy.r;
    const float x = result_yxy.g;
    const float y = result_yxy.b;

    const float3 result_xyz = float3(x * (Y / y), Y, (1.0f - x - y) * (Y / y));
    
    // Conversion from XYZ to RGB (https://en.wikipedia.org/wiki/SRGB#The_sRGB_transfer_function_.28.22gamma.22.29).
    const float r = dot(float3(3.2406f,  -1.5372f, -0.4986f), result_xyz);
    const float g = dot(float3(-0.9689f, 1.8758f,  0.0415f), result_xyz);
    const float b = dot(float3(0.0557f, -0.2040f, 1.0570f), result_xyz);

    return float3(r, g, b) * magnitude_multiplier;
}   

#endif