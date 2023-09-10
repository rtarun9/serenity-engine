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
    uint position_buffer_srv_index;
    uint texture_coord_buffer_srv_index;
    uint transform_buffer_cbv_index;
    uint scene_buffer_cbv_index;
    uint material_buffer_cbv_index;
};

// NOTE : For now atmoshpere computation is done herin post process combine shader itself. This is for experiemental purposes and will be changed in future.
struct PostProcessCombineRenderResources
{
    uint atmosphere_buffer_cbv_index;
    uint render_texture_srv_index;
};

struct AtmosphereRenderResources
{
    uint atmosphere_buffer_cbv_index;
};

#endif