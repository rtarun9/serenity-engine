// clang-format off

#ifndef __CONSTANT_BUFFERS__HLSLI__
#define __CONSTANT_BUFFERS__HLSLI__

#ifdef __cplusplus

#define uint uint32_t

#define float4x4 math::XMMATRIX

#define float2 math::XMFLOAT2
#define float3 math::XMFLOAT3
#define float4 math::XMFLOAT4

#define ConstantBufferStruct struct alignas(256) 

#else

#pragma pack_matrix(row_major)
#define ConstantBufferStruct struct

#endif

namespace interop
{
    static const uint INVALID_INDEX_U32 = -1;
    static const uint MAX_LIGHT_COUNT = 25u;
    static const uint SUN_LIGHT_INDEX = 0u;
    
    // Set the matrix packing to row major by default. Prevents needing to transpose matrices on the C++ side.
    struct TransformBuffer
    {
        float4x4 model_matrix;
        float4x4 transposed_inverse_model_matrix;
    };

    struct GameObjectBuffer
    {        
        TransformBuffer transform_buffer;
    };

    struct MeshBuffer
    {
        uint mesh_index;
        uint game_object_index;

        uint start_vertex_position;
        uint start_vertex_normal;
        uint start_vertex_texture_coord;

        uint start_index;
        uint indices_count;

        uint material_index;
    };
       
    // Light related data.
    enum class LightType
    {
        Point,
        Directional
    };

    // scale is only applicable for point light visualization. 
    struct Light
    {
        LightType light_type;
        float3 padding;

        float3 world_space_position_or_direction;
        float padding2;

        float3 color;
        float intensity;

        float scale;
        float3 padding4;
    };

    // NOTE : The light at index 0 will ALWAYS be a directional light, whose direction will be controlled
    // by the sun_angle field in light buffer.
    // This is just for convinence, and plus there probably wont be more than one directional light in a scene ever.
    // The sun_angle will be in degrees and not in radians.

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

    struct MaterialBuffer
    {
        float4 base_color;
        
        float2 metallic_roughness_factor;
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