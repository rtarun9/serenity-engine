#include "serenity-engine/core/application.hpp"

class Game final : public serenity::core::Application
{
  public:
    explicit Game()
    {
        using namespace serenity::graphics;

        const auto vertex_shader =
            ShaderCompiler::instance().compile(ShaderTypes::Vertex, L"shaders/triangle.hlsl", L"vs_main");

        const auto pixel_shader =
            ShaderCompiler::instance().compile(ShaderTypes::Pixel, L"shaders/triangle.hlsl", L"ps_main");

        // Create buffers.
        const auto triangle_position_buffer = std::array<math::XMFLOAT3, 3u>{
            math::XMFLOAT3{-0.5f, -0.5f, 0.0f},
            math::XMFLOAT3{0.0f, 0.5f, 0.0f},
            math::XMFLOAT3{0.5f, -0.5f, 0.0f},
        };

        m_triangle_position_buffer = Device::instance().create_buffer<math::XMFLOAT3>(
            BufferCreationDesc{
                .usage = BufferUsage::StructuredBuffer,
                .name = L"Triangle Position Buffer",
            },
            triangle_position_buffer);

        const auto triangle_color_buffer = std::array<math::XMFLOAT3, 3u>{
            math::XMFLOAT3{1.0f, 0.0f, 0.0f},
            math::XMFLOAT3{0.0f, 1.0f, 0.0f},
            math::XMFLOAT3{0.0f, 0.0f, 1.0f},
        };

        m_triangle_color_buffer = Device::instance().create_buffer<math::XMFLOAT3>(
            BufferCreationDesc{
                .usage = BufferUsage::StructuredBuffer,
                .name = L"Triangle Color Buffer",
            },
            triangle_color_buffer);

        constexpr auto triangle_index_buffer = std::array<uint16_t, 3u>{0u, 1u, 2u};

        m_triangle_index_buffer = Device::instance().create_buffer<uint16_t>(
            BufferCreationDesc{
                .usage = BufferUsage::IndexBuffer,
                .name = L"Triangle index buffer",
            },
            triangle_index_buffer);

        m_triangle_pipeline = Device::instance().create_pipeline(PipelineCreationDesc{
            .vertex_shader = vertex_shader,
            .pixel_shader = pixel_shader,
            .name = L"Triangle pixel shader",
        });
    }

    ~Game() = default;

    virtual void update(const float delta_time) override
    {
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
            std::array{cos(m_frame_index / 90.0f), 0.2f, abs(sinf(m_frame_index / 120.0f)), 1.0f});

        // Record the rendering related code.
        command_list.set_render_targets(std::array{back_buffer.descriptor_handle});
        command_list.set_descriptor_heaps(std::array{&graphics_device.get_cbv_srv_uav_descriptor_heap()});

        command_list.set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        command_list.set_index_buffer(m_triangle_index_buffer);

        command_list.set_bindless_graphics_root_signature();
        command_list.set_pipeline_state(m_triangle_pipeline);

        struct RenderResources
        {
            uint32_t position_buffer_index;
            uint32_t color_buffer_index;
        };

        const auto render_resources = RenderResources{
            .position_buffer_index = m_triangle_position_buffer.srv_index,
            .color_buffer_index = m_triangle_color_buffer.srv_index,
        };

        command_list.set_graphics_32_bit_root_constants(reinterpret_cast<const std::byte*>(&render_resources));

        const auto viewport = swapchain.get_viewport();
        const auto scissor_rect = swapchain.get_scissor_rect();

        command_list.set_viewport_and_scissor_rect(viewport, scissor_rect);

        command_list.draw_instanced(3u, 1u);

        // Transition backbuffer from render target to presentation.
        command_list.add_resource_barrier(back_buffer.resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
                                          D3D12_RESOURCE_STATE_PRESENT);
        command_list.execute_barriers();

        // Execute command list.
        graphics_device.get_direct_command_queue().execute(std::array{&command_list});

        swapchain.present();

        graphics_device.frame_end();

        ++m_frame_index;
    }

  private:
    serenity::graphics::Buffer m_triangle_position_buffer{};
    serenity::graphics::Buffer m_triangle_color_buffer{};
    serenity::graphics::Buffer m_triangle_index_buffer{};

    serenity::graphics::Pipeline m_triangle_pipeline{};
};

std::unique_ptr<serenity::core::Application> serenity::core::create_application()
{
    return std::make_unique<Game>();
}