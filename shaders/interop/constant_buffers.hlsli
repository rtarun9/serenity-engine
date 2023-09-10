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
    float4x4 view_projection_matrix;
};

ConstantBufferStruct MaterialBuffer
{
    float4 base_color;
    uint albedo_texture_srv_index;
};

#endif