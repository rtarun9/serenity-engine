// clang-format off

#ifndef __CONSTANT_BUFFERS_HLSLI__
#define __CONSTANT_BUFFERS_HLSLI__

#ifdef __cplusplus

#define uint uint32_t

#define float4x4 math::XMMATRIX

#define float3 math::XMFLOAT3
#define float4 math::XMFLOAT4

#define ConstantBufferStruct struct alignas(256) 

#else

#pragma pack_matrix(row_major)
#define ConstantBufferStruct struct

#endif

static const uint INVALID_INDEX_U32 = -1;

// Set the matrix packing to row major by default. Prevents needing to transpose matrices on the C++ side.

ConstantBufferStruct TransformBuffer
{
    float4x4 model_matrix;
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

    float3 sun_direction;
    float sun_angle;
};

ConstantBufferStruct MaterialBuffer
{
    float4 base_color;
    uint albedo_texture_srv_index;
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

    // Zenith luminance : Luminance at the Zenith (the imaginary point directly above the viewer).
    // float3 is of the form Yxy, where Y is the luminance, x and y are the chromaticities.
    float3 zenith_luminance_chromaticity;
    float padding2;

    float magnitude_multiplier;
    float3 padding3;

    PerezParameters perez_parameters;
};

#endif