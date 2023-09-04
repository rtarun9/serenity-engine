#include "serenity-engine/core/application.hpp"

#include "serenity-engine/scene/camera.hpp"

#include "serenity-engine/asset/mesh_loader.hpp"
#include "serenity-engine/scene/mesh.hpp"

#include "shaders/interop/constant_buffers.hlsli"
#include "shaders/interop/render_resources.hlsli"


class Game final : public serenity::core::Application
{
  public:
    explicit Game() 
    {
        using namespace serenity::graphics;

        const auto vertex_shader =
            ShaderCompiler::instance().compile(ShaderTypes::Vertex, L"shaders/mesh_viewer.hlsl", L"vs_main");

        const auto pixel_shader =
            ShaderCompiler::instance().compile(ShaderTypes::Pixel, L"shaders/mesh_viewer.hlsl", L"ps_main");

        m_cube = serenity::asset::MeshLoader::instance().load_mesh("data/Cube/glTF/Cube.gltf");

        m_transform_buffer = Device::instance().create_buffer<TransformBuffer>(BufferCreationDesc{
            .usage = BufferUsage::ConstantBuffer,
            .name = L"Transform Buffer",
        });

        m_pipeline = Device::instance().create_pipeline(PipelineCreationDesc{
            .vertex_shader = vertex_shader,
            .pixel_shader = pixel_shader,
            .name = L"Mesh Viewer pixel shader",
        });

        m_depth_texture = Device::instance().create_texture(TextureCreationDesc{
            .usage = TextureUsage::Depth,
            .format = DXGI_FORMAT_D32_FLOAT,
            .dimension =
                serenity::Uint2{
                    .x = 1080u,
                    .y = 720u,
                },
            .name = L"Depth Texture",

        });
    }

    ~Game() = default;

    virtual void update(const float delta_time) override
    {
        m_camera.update(delta_time, m_input);

        const auto model_matrix = math::XMMatrixIdentity();

        const auto window_dimensions = m_window->get_dimensions();
        const auto aspect_ratio = static_cast<float>(window_dimensions.x) / static_cast<float>(window_dimensions.y);

        auto transform_buffer_data = TransformBuffer{
            .mvp_matrix = model_matrix * m_camera.get_view_matrix() *
                          math::XMMatrixPerspectiveFovLH(math::XMConvertToRadians(45.0f), aspect_ratio, 0.1f, 100.0f),
        };

        m_transform_buffer.update(reinterpret_cast<std::byte *>(&transform_buffer_data), sizeof(TransformBuffer));
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
        command_list.set_index_buffer(m_cube.value().index_buffer);

        command_list.set_bindless_graphics_root_signature();
        command_list.set_pipeline_state(m_pipeline);

        const auto render_resources = MeshViewerRenderResources{
            .position_buffer_index = m_cube.value().position_buffer.srv_index,
            .transform_buffer_index = m_transform_buffer.cbv_index,
        };

        command_list.set_graphics_32_bit_root_constants(reinterpret_cast<const std::byte *>(&render_resources));

        const auto viewport = swapchain.get_viewport();
        const auto scissor_rect = swapchain.get_scissor_rect();

        command_list.set_viewport_and_scissor_rect(viewport, scissor_rect);

        command_list.draw_indexed_instanced(m_cube.value().indices_count, 1u);

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
    std::optional<serenity::scene::Mesh> m_cube{};

    serenity::graphics::Pipeline m_pipeline{};

    serenity::graphics::Buffer m_transform_buffer{};

    serenity::graphics::Texture m_depth_texture{};

    serenity::scene::Camera m_camera{};
};

std::unique_ptr<serenity::core::Application> serenity::core::create_application()
{
    return std::make_unique<Game>();
}