// clang-format off

#ifndef __STRUCTURED_BUFFERS_HLSLI__
#define __STRUCTURED_BUFFERS_HLSLI__

#include "interop_common.hlsli"

namespace interop
{
    struct TransformBuffer
    {
        float4x4 model_matrix;
        float4x4 inverse_model_matrix;
    };

    struct GameObjectBuffer
    {        
        TransformBuffer transform_buffer;
    };

    struct MeshBuffer
    {
        uint mesh_index;
        uint game_object_index;

        uint position_offset;
        uint normal_offset;
        uint texture_coord_offset;

        uint indices_offset;
        uint indices_count;

        uint padding;
        
        float4x4 mesh_local_transform_matrix;
        float4x4 inverse_mesh_local_transform_matrix;
        
        uint material_index;
        float3 padding2;
        float4 padding3;
    };

    struct MaterialBuffer
    {
        float4 base_color;
        
        float2 metallic_roughness_factor;
        uint albedo_texture_srv_index;
    };
       
}
#endif