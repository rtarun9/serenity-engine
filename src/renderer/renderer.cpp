#include "serenity-engine/renderer/renderer.hpp"

#include "serenity-engine/editor/editor.hpp"
#include "serenity-engine/scene/scene_manager.hpp"

#include "shaders/interop/render_resources.hlsli"

namespace serenity::renderer
{
    Renderer::Renderer(window::Window &window) : window_ref(window)
    {
        // Create resources.
        m_shader_compiler = std::make_unique<ShaderCompiler>();

        m_device = std::make_unique<rhi::Device>(window.get_window_handle(), window.get_dimensions());

        create_resources();
        create_renderpasses();

        core::Log::instance().info("Created renderer");
    }

    Renderer::~Renderer()
    {
        core::Log::instance().info("Destroyed renderer");
    }

    void Renderer::render()
    {
        auto &graphics_device = (*m_device.get());
        auto &swapchain = m_device->get_swapchain();

        // Frame start will reset the command list and command allocator associated with the current frame.
        graphics_device.frame_start();

        auto &command_list = graphics_device.get_current_frame_direct_command_list();
        auto &back_buffer = swapchain.get_current_back_buffer();

        const auto back_buffer_index = swapchain.get_current_backbuffer_index();

        // Transition back buffer from presentation mode to render target format.
        command_list.add_resource_barrier(back_buffer.resource.Get(), D3D12_RESOURCE_STATE_PRESENT,
                                          D3D12_RESOURCE_STATE_RENDER_TARGET);
        command_list.execute_barriers();

        // Clear rtv and set new clear color.
        command_list.clear_render_target_views(back_buffer.descriptor_handle, std::array{0.0f, 0.0f, 0.0f, 1.0f});

        // Record the rendering related code.
        const auto dsv_descriptor =
            graphics_device.get_dsv_descriptor_heap().get_handle_at_index(m_depth_texture.dsv_index);
        command_list.clear_depth_stencil_view(dsv_descriptor, 1.0f);

        command_list.set_render_targets(std::array{back_buffer.descriptor_handle}, dsv_descriptor);
        command_list.set_descriptor_heaps(std::array{&graphics_device.get_cbv_srv_uav_descriptor_heap()});

        command_list.set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        const auto viewport = swapchain.get_viewport();
        const auto scissor_rect = swapchain.get_scissor_rect();

        command_list.set_viewport_and_scissor_rect(viewport, scissor_rect);

        command_list.set_bindless_graphics_root_signature();
        command_list.set_pipeline_state(m_pipeline);

        const auto &scene_buffer_index = scene::SceneManager::instance().get_current_scene().get_scene_buffer_index();

        for (auto models = scene::SceneManager::instance().get_current_scene().get_models(); auto &model : models)
        {
            for (const auto &mesh : model.meshes)
            {
                command_list.set_index_buffer(get_buffer_at_index(mesh.index_buffer_index));

                const auto render_resources = MeshViewerRenderResources{
                    .position_buffer_srv_index = get_buffer_at_index(mesh.position_buffer_index).srv_index,
                    .texture_coord_buffer_srv_index = get_buffer_at_index(mesh.texture_coords_buffer_index).srv_index,
                    .transform_buffer_cbv_index =
                        get_buffer_at_index(model.transform_component.transform_buffer_index).cbv_index,
                    .scene_buffer_cbv_index = get_buffer_at_index(scene_buffer_index).cbv_index,
                    .material_buffer_cbv_index =
                        get_buffer_at_index(model.materials.at(mesh.material_index).material_buffer_index).cbv_index,
                };

                command_list.set_graphics_32_bit_root_constants(reinterpret_cast<const std::byte *>(&render_resources));
                command_list.draw_indexed_instanced(mesh.indices, 1u);
            };
        }

        editor::Editor::instance().render();

        // Transition backbuffer from render target to presentation.
        command_list.add_resource_barrier(back_buffer.resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
                                          D3D12_RESOURCE_STATE_PRESENT);
        command_list.execute_barriers();

        // Execute command list.
        graphics_device.get_direct_command_queue().execute(std::array{&command_list});

        swapchain.present();

        graphics_device.frame_end();
    }

    void Renderer::create_resources()
    {
        // Create depth texture.
        m_depth_texture = m_device->create_texture(renderer::rhi::TextureCreationDesc{
            .usage = renderer::rhi::TextureUsage::DepthStencilTexture,
            .format = DXGI_FORMAT_D32_FLOAT,
            .dimension = window_ref.get_dimensions(),
            .name = L"Depth Texture",

        });

        // Create pipeline.
        m_pipeline = m_device->create_pipeline(renderer::rhi::PipelineCreationDesc{
            .vertex_shader = renderer::ShaderCompiler::instance().compile(renderer::ShaderTypes::Vertex,
                                                                          L"shaders/mesh_viewer.hlsl", L"vs_main"),
            .pixel_shader = renderer::ShaderCompiler::instance().compile(renderer::ShaderTypes::Pixel,
                                                                         L"shaders/mesh_viewer.hlsl", L"ps_main"),
            .dsv_format = DXGI_FORMAT_D32_FLOAT,
            .name = L"Mesh Viewer pixel shader",
        });
    }

    void Renderer::create_renderpasses()
    {
    }
} // namespace serenity::renderer