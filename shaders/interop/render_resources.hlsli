// clang-format off

#ifndef __RENDER_RESOURCES_HLSLI__
#define __RENDER_RESOURCES_HLSLI__

#ifdef __cplusplus

#define uint uint32_t

#endif

struct TriangleRenderResources
{
    uint position_buffer_index;
    uint color_buffer_index;
    uint transform_buffer_index;
};

struct MeshViewerRenderResources
{
    uint position_buffer_index;
    uint color_buffer_index;
    uint transform_buffer_index;
};

#endif