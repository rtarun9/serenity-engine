// clang-format off

// Contains variables that are shared between C++ and HLSL.
// Also contains the datatype translations of C++ <-> HLSL shared variables.  

#ifndef __INTEROP_COMMON_HLSLI__
#define __INTEROP_COMMON_HLSLI__

#ifdef __cplusplus

#define uint uint32_t

#define float4x4 math::XMMATRIX

#define float2 math::XMFLOAT2
#define float3 math::XMFLOAT3
#define float4 math::XMFLOAT4

#define ConstantBufferStruct struct alignas(256) 

#else

// Set the matrix packing to row major by default. Prevents needing to transpose matrices on the C++ side.
#pragma pack_matrix(row_major)
#define ConstantBufferStruct struct

#endif

namespace interop
{
    static const uint INVALID_INDEX_U32 = -1;
    static const uint MAX_LIGHT_COUNT = 25u;
    static const uint SUN_LIGHT_INDEX = 0u;
}

#endif