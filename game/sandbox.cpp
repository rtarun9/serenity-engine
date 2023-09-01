#include "serenity-engine/core/application.hpp"

class Game final : public serenity::core::Application
{
  public:
    explicit Game() = default;
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
        command_list.get_command_list()->ClearRenderTargetView(
            back_buffer.descriptor_handle.cpu_descriptor_handle,
            std::array{cos(m_frame_index / 90.0f), 0.2f, abs(sinf(m_frame_index / 120.0f)), 1.0f}.data(), 0u, nullptr);

        // Transition backbuffer from render target to presentation.
        command_list.add_resource_barrier(back_buffer.resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
                                          D3D12_RESOURCE_STATE_PRESENT);
        command_list.execute_barriers();

        // Execute command list.
        const auto command_list_for_execution = std::array{&command_list};
        graphics_device.get_direct_command_queue().execute(command_list_for_execution);

        swapchain.present();

        graphics_device.frame_end();

        ++m_frame_index;
    }
};

std::unique_ptr<serenity::core::Application> serenity::core::create_application()
{
    return std::make_unique<Game>();
}