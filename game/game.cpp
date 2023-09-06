#include "serenity-engine/core/application.hpp"

#include "serenity-engine/asset/texture_loader.hpp"

#include "shaders/interop/render_resources.hlsli"

using namespace serenity;

class Game final : public core::Application
{

  public:
    explicit Game()
    {
        // Create depth texture.
        m_depth_texture = graphics::Device::instance().create_texture(graphics::TextureCreationDesc{
            .usage = graphics::TextureUsage::DepthStencilTexture,
            .format = DXGI_FORMAT_D32_FLOAT,
            .dimension = m_window->get_dimensions(),
            .name = L"Depth Texture",

        });

        // Create pipeline.
        m_pipeline = graphics::Device::instance().create_pipeline(graphics::PipelineCreationDesc{
            .vertex_shader = graphics::ShaderCompiler::instance().compile(graphics::ShaderTypes::Vertex,
                                                                          L"shaders/mesh_viewer.hlsl", L"vs_main"),
            .pixel_shader = graphics::ShaderCompiler::instance().compile(graphics::ShaderTypes::Pixel,
                                                                         L"shaders/mesh_viewer.hlsl", L"ps_main"),
            .name = L"Mesh Viewer pixel shader",
        });

        // Create albedo texture and scene.
        auto sample_texture_data = asset::TextureLoader::load_texture("data/Cube/glTF/Cube_BaseColor.png");
        auto &texture_data_vector = std::get<std::vector<uint8_t>>(sample_texture_data.data);

        auto default_scene = scene::Scene("Default Scene");
        default_scene.add_model("data/Cube/glTF/Cube.gltf", "Cube");

        scene::SceneManager::instance().add_scene(std::move(default_scene));

        m_albedo_texture = graphics::Device::instance().create_texture(
            graphics::TextureCreationDesc{
                .usage = graphics::TextureUsage::ShaderResourceTexture,
                .format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .bytes_per_pixel = 4u,
                .dimension = sample_texture_data.dimension,
            },
            reinterpret_cast<const std::byte *>(texture_data_vector.data()));
    }

    ~Game() = default;

    virtual void update(const float delta_time) override
    {
        // Update scene objects (camera and models).
        auto &current_scene_camera = scene::SceneManager::instance().get_current_scene().get_camera();
        current_scene_camera.update(delta_time, m_input);

        const auto window_dimensions = m_window->get_dimensions();
        const auto aspect_ratio = static_cast<float>(window_dimensions.x) / static_cast<float>(window_dimensions.y);

        const auto projection_matrix =
            math::XMMatrixPerspectiveFovLH(math::XMConvertToRadians(45.0f), aspect_ratio, 0.1f, 100.0f);

        scene::SceneManager::instance().get_current_scene().update(projection_matrix);
    }

    virtual void render() override
    {
        auto &graphics_device = serenity::graphics::Device::instance();
        auto &swapchain = serenity::graphics::Swapchain::instance();

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
        command_list.clear_render_target_views(
            back_buffer.descriptor_handle,
            std::array{cos(m_frame_count / 90.0f), 0.2f, abs(sinf(m_frame_count / 120.0f)), 1.0f});

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

        for (auto models = scene::SceneManager::instance().get_current_scene().get_models(); auto &model : models)
        {
            for (const auto &mesh : model.meshes)
            {
                command_list.set_index_buffer(mesh.index_buffer);

                const auto render_resources = MeshViewerRenderResources{
                    .position_buffer_index = mesh.position_buffer.srv_index,
                    .texture_coord_buffer_index = mesh.texture_coords_buffer.srv_index,
                    .albedo_texture_index = m_albedo_texture.srv_index,
                    .transform_buffer_index = model.transform_component.transform_buffer.cbv_index,
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

        ++m_frame_count;
    }

  private:
    graphics::Pipeline m_pipeline{};

    graphics::Texture m_depth_texture{};
    graphics::Texture m_albedo_texture{};
};

std::unique_ptr<serenity::core::Application> serenity::core::create_application()
{
    return std::make_unique<Game>();
}