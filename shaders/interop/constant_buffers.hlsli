// clang-format off

#ifndef __CONSTANT_BUFFERS_HLSLI__
#define __CONSTANT_BUFFERS_HLSLI__

#include "interop_common.hlsli"

namespace interop
{    
    // Light related data.
    enum class LightType
    {
        Point,
        Directional
    };

    // Note : There will be only one directional light in the scene, at index 0 in
    // the light buffers lights array.
    // scale_or_sun_angle for point lights is scale of visualization model.
    // scale_or_sun_angle for directional lights is the sun angle.
    struct Light
    {
        LightType light_type;
        float3 padding;

        float3 world_space_position_or_direction;
        float padding2; 

        float3 color;
        float intensity;

        float scale_or_sun_angle;
        float3 padding4;
    };

    // model_matrix is kept seperate from the lights field because instanced rendering 
    // will be used to render non-directional lights.
    // Index 0 in the model_matrix array corresponds to index 1 in the lights array.
    ConstantBufferStruct LightBuffer
    {
        uint light_count;
        float sun_angle;
        float2 padding;

        Light lights[MAX_LIGHT_COUNT];

        float4x4 model_matrix[MAX_LIGHT_COUNT];
    };

    ConstantBufferStruct SceneBuffer
    {
        float4x4 view_matrix;
        float4x4 inverse_view_matrix;
        float4x4 view_projection_matrix;
        float4x4 inverse_view_projection_matrix;
        float4x4 inverse_projection_matrix;

        float3 camera_position;
        float padding;
    };

    // The float3's A, B, C, D, E are part of the parameters required by the Perez et.al model to compute sky
    // luminance distribution. The other parameters are theta and gamma.
    // A corresponds to darkening / brightening of horizon.
    // B corresponds to luminance gradient near the horizon.
    // C corresponds to relative intensity of circumsolar region.
    // D corresponds to width of the circumsolar region.
    // E corresponds to relative backscattered light.
    // These terms can be computed with the help of turbidity value (as mentioned in the A.J Preetham paper section
    // A.2)
    struct PerezParameters
    {
        float3 A;
        float padding1;

        float3 B;
        float padding2;

        float3 C;
        float padding3;

        float3 D;
        float padding4;

        float3 E;
        float padding5;
    };

    // Parameters required by the Preetham sky analytical model for daylight' implementation.
    ConstantBufferStruct AtmosphereRenderPassBuffer
    {
        // Turbidity : Fraction of scattering due to haze as opposed to just molecules.
        float turbidity;
        float3 padding;

        float2 output_texture_dimension;
        float2 padding2;

        // Zenith luminance : Luminance at the Zenith (the imaginary point directly above the viewer).
        // float3 is of the form Yxy, where Y is the luminance, x and y are the chromaticities.
        float3 zenith_luminance_chromaticity;
        float padding3;
        
        float magnitude_multiplier;
        float3 padding4;

        PerezParameters perez_parameters;
    };
    
    ConstantBufferStruct PostProcessBuffer
    {
        float2 screen_dimensions;
        float2 padding;
    
        uint frame_count;
        float3 padding2;
        
        float noise_scale;
        float3 padding3;
    };
}

#endif