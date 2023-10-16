#include "serenity-engine/renderer/renderpass/post_processing_renderpass.hpp"

#include "serenity-engine/renderer/renderer.hpp"

#include "shaders/interop/render_resources.hlsli"

namespace serenity::renderer::renderpass
{
    PostProcessingRenderpass::PostProcessingRenderpass()
    {
        core::Log::instance().info("Created post processing render pass");

        // Create pipeline objects.

        m_post_process_pipeline_index = renderer::Renderer::instance().create_pipeline(rhi::PipelineCreationDesc{
            .vertex_shader_creation_desc =
                ShaderCreationDesc{
                    .shader_type = ShaderTypes::Vertex,
                    .shader_path = L"shaders/post_process.hlsl",
                    .shader_entry_point = L"vs_main",
                },
            .pixel_shader_creation_desc =
                ShaderCreationDesc{
                    .shader_type = ShaderTypes::Pixel,
                    .shader_path = L"shaders/post_process.hlsl",
                    .shader_entry_point = L"ps_main",
                },
            .rtv_formats = {rhi::Swapchain::SWAPCHAIN_RTV_FORMAT},
            .dsv_format = DXGI_FORMAT_UNKNOWN,
            .name = L"Post process pipeline",
        });

        m_fullscreen_triangle_index_buffer_index = renderer::Renderer::instance().create_buffer<uint32_t>(
            rhi::BufferCreationDesc{
                .usage = rhi::BufferUsage::IndexBuffer,
                .name = L"Full screen triangle index buffer",
            },
            std::array{0u, 1u, 2u});

        // Create post process buffer data.
        m_post_process_buffer_index =
            renderer::Renderer::instance().create_buffer<interop::PostProcessBuffer>(rhi::BufferCreationDesc{
                .usage = rhi::BufferUsage::ConstantBuffer,
                .name = L"Post Process Buffer",
            });

        m_post_process_buffer_data.noise_scale = 1.0f / 1024.0f;
    }

    PostProcessingRenderpass::~PostProcessingRenderpass()
    {
        core::Log::instance().info("Destroyed post processing render pass");
    }

    void PostProcessingRenderpass::render(rhi::CommandList &command_list, const uint32_t render_texture_srv_index) const
    {
        // Set pipeline and root signature state.
        command_list.set_bindless_graphics_root_signature();
        command_list.set_pipeline_state(
            renderer::Renderer::instance().get_pipeline_at_index(m_post_process_pipeline_index));

        auto post_process_combine_render_resources = interop::PostProcessRenderResources{
            .render_texture_srv_index = render_texture_srv_index,
            .post_process_buffer_cbv_index =
                renderer::Renderer::instance().get_buffer_at_index(m_post_process_buffer_index).cbv_index,
        };

        command_list.set_index_buffer(
            renderer::Renderer::instance().get_buffer_at_index(m_fullscreen_triangle_index_buffer_index));

        command_list.set_graphics_32_bit_root_constants(
            reinterpret_cast<const std::byte *>(&post_process_combine_render_resources));

        command_list.draw_instanced(3u, 1u);
    }
} // namespace serenity::renderer::renderpass