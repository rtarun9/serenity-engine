#include "serenity-engine/renderer/renderpass/shading_renderpass.hpp"

#include "serenity-engine/renderer/renderer.hpp"
#include "serenity-engine/scene/scene_manager.hpp"

#include "shaders/interop/render_resources.hlsli"

namespace serenity::renderer::renderpass
{
    ShadingRenderpass::ShadingRenderpass()
    {
        core::Log::instance().info("Created shading render pass");

        // Create pipeline objects.
        m_shading_pipeline_index = Renderer::instance().create_pipeline(rhi::PipelineCreationDesc{
            .pipeline_variant = rhi::PipelineVariant::Graphics,
            .vertex_shader_creation_desc =
                ShaderCreationDesc{
                    .shader_type = ShaderTypes::Vertex,
                    .shader_path = L"shaders/shading/pbr.hlsl",
                    .shader_entry_point = L"vs_main",
                },
            .pixel_shader_creation_desc =
                ShaderCreationDesc{
                    .shader_type = ShaderTypes::Pixel,
                    .shader_path = L"shaders/shading/pbr.hlsl",
                    .shader_entry_point = L"ps_main",
                },
            .cull_mode = D3D12_CULL_MODE_BACK,
            .rtv_formats = {DXGI_FORMAT_R16G16B16A16_FLOAT},
            .dsv_format = DXGI_FORMAT_D32_FLOAT,
            .name = L"PBR Shading Pipeline",
        });
    }

    ShadingRenderpass::~ShadingRenderpass()
    {
        core::Log::instance().info("Destroyed shading render pass");
    }

    void ShadingRenderpass::render(rhi::CommandList &command_list, rhi::CommandSignature &command_signature,
                                   const uint32_t command_buffer_index,
                                   const uint32_t scene_buffer_cbv_index,
                                   const uint32_t atmosphere_texture_srv_index) const
    {
        // Render scene objects.

        // Set pipeline and root signature state.
        command_list.set_bindless_graphics_root_signature();
        command_list.set_pipeline_state(renderer::Renderer::instance().get_pipeline_at_index(m_shading_pipeline_index));

        auto &current_scene = scene::SceneManager::instance().get_current_scene();

        const auto get_buffer_at_index = [&](const uint32_t index) {
            return renderer::Renderer::instance().get_buffer_at_index(index);
        };

        const auto get_texture_at_index = [&](const uint32_t index) {
            return renderer::Renderer::instance().get_texture_at_index(index);
        };

        command_list.set_index_buffer(get_buffer_at_index(current_scene.m_scene_index_buffer_index));
        const auto render_resources = interop::PBRShadingRenderResources{
            .position_buffer_srv_index = get_buffer_at_index(current_scene.m_scene_position_buffer_index).srv_index,
            .texture_coord_buffer_srv_index =
                get_buffer_at_index(current_scene.m_scene_texture_coords_buffer_index).srv_index,
            .normal_buffer_srv_index = get_buffer_at_index(current_scene.m_scene_normal_buffer_index).srv_index,
            .game_object_cbv_index = get_buffer_at_index(current_scene.m_scene_game_object_buffer_index).srv_index,
            .scene_buffer_cbv_index = get_buffer_at_index(current_scene.get_scene_buffer_index()).cbv_index,
            .light_buffer_cbv_index =
                get_buffer_at_index(current_scene.get_lights().get_light_buffer_index()).cbv_index,
            .material_buffer_cbv_index = get_buffer_at_index(current_scene.m_scene_materal_buffer_index).srv_index,
            .atmosphere_texture_srv_index = atmosphere_texture_srv_index,
        };


        command_signature.m_indirect_commands.clear();

        //for (auto game_objects = current_scene.get_game_objects();
         //                                       auto &[name, game_object] : game_objects)
        {
            for (const auto &mesh : current_scene.m_mesh_parts)
            {
                command_signature.m_indirect_commands.emplace_back(
                    rhi::IndirectCommand{.object_id = mesh.game_object_index,
                                         .draw_arguments = {
                                             .IndexCountPerInstance = mesh.indices_count,
                                             .InstanceCount = 1u,
                                             .StartIndexLocation = mesh.start_index,
                                             .BaseVertexLocation = 0u,
                                             .StartInstanceLocation = 0u,
                                         },});
            };
        }
        
        command_list.set_graphics_32_bit_root_constants(reinterpret_cast<const std::byte *>(&render_resources));
        
        renderer::Renderer::instance()
            .get_buffer_at_index(command_buffer_index)
            .update(reinterpret_cast<const std::byte *>(command_signature.m_indirect_commands.data()),
                    command_signature.m_indirect_commands.size() * sizeof(rhi::IndirectCommand));

        command_list.get_command_list()->ExecuteIndirect(
            command_signature.m_command_signature.Get(), command_signature.m_indirect_commands.size(),
            get_buffer_at_index(command_buffer_index).resource.Get(), 0u, nullptr, 0u);
    }
} // namespace serenity::renderer::renderpass