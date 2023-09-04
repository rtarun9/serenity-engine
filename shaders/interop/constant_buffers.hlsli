// clang-format off

#ifndef __CONSTANT_BUFFERS_HLSLI__
#define __CONSTANT_BUFFERS_HLSLI__

#ifdef __cplusplus

#define uint uint32_t
#define float4x4 math::XMMATRIX
#define ConstantBufferStruct struct alignas(256) 

#else

#pragma pack_matrix(row_major)
#define ConstantBufferStruct struct

#endif


// Set the matrix packing to row major by default. Prevents needing to transpose matrices on the C++ side.

ConstantBufferStruct TransformBuffer
{
	float4x4 mvp_matrix;
};

#endif