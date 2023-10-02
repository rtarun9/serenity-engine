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

struct PhongShadingRenderResources
{
    uint position_buffer_srv_index;
    uint texture_coord_buffer_srv_index;
    uint normal_buffer_srv_index;
    uint transform_buffer_cbv_index;
    uint scene_buffer_cbv_index;
    uint light_buffer_cbv_index;
    uint material_buffer_cbv_index;
    uint atmosphere_texture_srv_index;
};

struct PostProcessCombineRenderResources
{
    uint render_texture_srv_index;
};

struct AtmosphereRenderResources
{
    uint light_buffer_cbv_index;
    uint atmosphere_buffer_cbv_index;
    uint output_texture_uav_index;
};

struct CubeMapRenderResources
{
    uint texture_srv_index;
    uint position_buffer_srv_index;
    uint scene_buffer_cbv_index;
};

struct LightRenderResources
{
    uint scene_buffer_cbv_index;
    uint light_buffer_cbv_index;
    uint light_cube_position_buffer_srv_index;
};
#endif