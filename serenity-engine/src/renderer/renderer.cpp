#include "serenity-engine/renderer/renderer.hpp"

#include "serenity-engine/editor/editor.hpp"
#include "serenity-engine/scene/scene_manager.hpp"

#include "serenity-engine/core/application.hpp"

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
        auto &device = (*m_device.get());
        auto &swapchain = m_device->get_swapchain();

        // Frame start will reset the command list and command allocator associated with the current frame.
        device.frame_start();

        auto &command_list = device.get_current_frame_direct_command_list();
        auto &back_buffer = swapchain.get_current_back_buffer();

        const auto back_buffer_index = swapchain.get_current_backbuffer_index();

        const auto render_texture_descriptor_handle =
            m_device->get_rtv_descriptor_heap().get_handle_at_index(m_render_texture.rtv_index);

        // Transition back buffer from presentation mode to render target format, and render texture from pixel shader
        // resource to render target.
        command_list.add_resource_barrier(back_buffer.resource.Get(), D3D12_RESOURCE_STATE_PRESENT,
                                          D3D12_RESOURCE_STATE_RENDER_TARGET);
        command_list.add_resource_barrier(m_render_texture.resource.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                          D3D12_RESOURCE_STATE_RENDER_TARGET);

        command_list.execute_barriers();

        // Render into the render texture, and then run the post process pipeline which will render into the swapchain
        // backbuffer.

        // Clear rtv and set clear color.
        command_list.clear_render_target_views(back_buffer.descriptor_handle, std::array{0.0f, 0.0f, 0.0f, 1.0f});
        command_list.clear_render_target_views(render_texture_descriptor_handle, std::array{0.0f, 0.0f, 0.0f, 1.0f});

        // Record the rendering related code.
        const auto dsv_descriptor = device.get_dsv_descriptor_heap().get_handle_at_index(m_depth_texture.dsv_index);
        command_list.clear_depth_stencil_view(dsv_descriptor, 1.0f);

        {
            command_list.set_render_targets(std::array{render_texture_descriptor_handle}, dsv_descriptor);
            command_list.set_descriptor_heaps(std::array{&device.get_cbv_srv_uav_descriptor_heap()});

            command_list.set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            const auto viewport = swapchain.get_viewport();
            const auto scissor_rect = swapchain.get_scissor_rect();

            command_list.set_viewport_and_scissor_rect(viewport, scissor_rect);

            const auto &scene_buffer_index =
                scene::SceneManager::instance().get_current_scene().get_scene_buffer_index();

            const auto &light_buffer_index =
                scene::SceneManager::instance().get_current_scene().get_lights().get_light_buffer_index();

            // Process the atmosphere render pass.
            {
                m_atmosphere_renderpass->compute(command_list, get_buffer_at_index(scene_buffer_index).cbv_index,
                                                 get_buffer_at_index(light_buffer_index).cbv_index);
            }

            // Render scene objects.
            {
                m_shading_renderpass->render(
                    command_list, m_command_signature.value(),
                    m_command_buffer_indices.at(m_device->get_swapchain().get_current_backbuffer_index()),
                    get_buffer_at_index(scene_buffer_index).cbv_index,
                    get_texture_at_index(m_atmosphere_renderpass->get_atmosphere_texture_index()).srv_index);
            }

            // Render lights.
            {
                auto &lights = scene::SceneManager::instance().get_current_scene().get_lights();
                lights.render(command_list, get_buffer_at_index(scene_buffer_index).cbv_index);
            }

            // Render cube map.
            {
                m_cube_map_renderpass->render(
                    command_list, get_buffer_at_index(scene_buffer_index).cbv_index,
                    get_texture_at_index(m_atmosphere_renderpass->get_atmosphere_texture_index()).srv_index);
            }
        }

        // Transition render texture to shader resource and perform the post process combine operations.
        command_list.add_resource_barrier(m_render_texture.resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
                                          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        command_list.execute_barriers();

        command_list.set_render_targets(std::array{back_buffer.descriptor_handle});

        {
            // Render post processing phase.
            m_post_processing_renderpass->render(command_list, m_command_signature.value(), m_render_texture.srv_index);

            // command_list.get_command_list()->ExecuteIndirect(m_command_signature.value().m_command_signature.Get(),
            // 1u, )
        }

        {
            editor::Editor::instance().render();
        }

        // Transition backbuffer from render target to presentation.
        command_list.add_resource_barrier(back_buffer.resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
                                          D3D12_RESOURCE_STATE_PRESENT);
        command_list.execute_barriers();

        // Execute command list.
        device.get_direct_command_queue().execute(std::array{&command_list});

        swapchain.present();

        device.frame_end();

        reload_pipelines();
    }

    void Renderer::update_renderpasses(const uint32_t frame_count)
    {
        const auto &light_buffer = scene::SceneManager::instance().get_current_scene().get_lights().get_light_buffer();

        m_atmosphere_renderpass->update(
            light_buffer.lights[interop::SUN_LIGHT_INDEX].world_space_position_or_direction);

        m_post_processing_renderpass->get_post_process_buffer().frame_count = frame_count;
        m_post_processing_renderpass->get_post_process_buffer().screen_dimensions = {
            static_cast<float>(window_ref.get_dimensions().x),
            static_cast<float>(window_ref.get_dimensions().y),
        };

        get_buffer_at_index(m_post_processing_renderpass->get_post_process_buffer_index())
            .update(reinterpret_cast<const std::byte *>(&m_post_processing_renderpass->get_post_process_buffer()),
                    sizeof(interop::PostProcessBuffer));
    }

    void Renderer::create_resources()
    {
        // Create command signature.
        m_command_signature = rhi::CommandSignature(m_device->get_device());

        // Create command buffers.
        for (auto &buffer_index : m_command_buffer_indices)
        {
            buffer_index = create_buffer<rhi::IndirectCommand>(
                rhi::BufferCreationDesc{
                    .usage = rhi::BufferUsage::DynamicStructuredBuffer,
                    .name = L"Command Buffer",
                },
                std::vector<rhi::IndirectCommand>(MAX_PRIMITIVE_COUNT));
        }

        // Create depth texture.
        m_depth_texture = m_device->create_texture(rhi::TextureCreationDesc{
            .usage = rhi::TextureUsage::DepthStencilTexture,
            .format = DXGI_FORMAT_D32_FLOAT,
            .dimension = window_ref.get_dimensions(),
            .name = L"Depth Texture",

        });

        // Create render texture.
        m_render_texture = m_device->create_texture(rhi::TextureCreationDesc{
            .usage = rhi::TextureUsage::RenderTexture,
            .format = DXGI_FORMAT_R16G16B16A16_FLOAT,
            .dimension = window_ref.get_dimensions(),
            .name = L"Render Texture",
        });
    }

    void Renderer::create_renderpasses()
    {
        m_atmosphere_renderpass = std::make_unique<renderpass::AtmosphereRenderpass>();
        m_cube_map_renderpass = std::make_unique<renderpass::CubeMapRenderpass>();
        m_shading_renderpass = std::make_unique<renderpass::ShadingRenderpass>();
        m_post_processing_renderpass = std::make_unique<renderpass::PostProcessingRenderpass>();
    }

    void Renderer::reload_pipelines()
    {
        for (const auto &index : m_pipeline_reload_buffer)
        {
            if (index >= m_pipelines.size())
            {
                core::Log::instance().warn(
                    std::format("{} was not a valid pipeline index. No further action is performed", index));
                return;
            }

            const auto pipeline = m_device->create_pipeline(m_pipelines[index].pipeline_creation_desc, true);
            if (pipeline.pipeline_state == nullptr)
            {
                core::Log::instance().warn(std::format("Failed to reload pipeline {}!",
                                                       wstring_to_string(pipeline.pipeline_creation_desc.name)));
            }
            else
            {
                m_pipelines[index] = pipeline;
            }
            m_pipelines[index].index = index;
        }

        m_pipeline_reload_buffer.clear();
    }
} // namespace serenity::renderer