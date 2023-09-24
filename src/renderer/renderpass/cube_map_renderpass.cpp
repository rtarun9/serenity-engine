#include "serenity-engine/renderer/renderpass/cube_map_renderpass.hpp"

#include "serenity-engine/renderer/renderer.hpp"

#include "shaders/interop/render_resources.hlsli"

namespace serenity::renderer::renderpass
{
    CubeMapRenderpass::CubeMapRenderpass()
    {
        core::Log::instance().info("Created cube map render pass");

        // Create pipeline objects.
        m_cubemap_pipeline_index = Renderer::instance().create_pipeline(rhi::PipelineCreationDesc{
            .pipeline_variant = rhi::PipelineVariant::Graphics,
            .vertex_shader_creation_desc =
                ShaderCreationDesc{
                    .shader_type = ShaderTypes::Vertex,
                    .shader_path = L"shaders/cube_map.hlsl",
                    .shader_entry_point = L"vs_main",
                },
            .pixel_shader_creation_desc =
                ShaderCreationDesc{
                    .shader_type = ShaderTypes::Pixel,
                    .shader_path = L"shaders/cube_map.hlsl",
                    .shader_entry_point = L"ps_main",
                },
            .cull_mode = D3D12_CULL_MODE_FRONT,
            .rtv_formats = {DXGI_FORMAT_R16G16B16A16_FLOAT},
            .dsv_format = DXGI_FORMAT_D32_FLOAT,
            .name = L"Cube map Pipeline",
        });

        // Load cubemap data (positions and indices).
        constexpr auto positions = std::array{
            math::XMFLOAT3(-1.0f, -1.0f, -1.0f), math::XMFLOAT3(-1.0f, +1.0f, -1.0f),
            math::XMFLOAT3(+1.0f, +1.0f, -1.0f), math::XMFLOAT3(+1.0f, -1.0f, -1.0f),
            math::XMFLOAT3(-1.0f, -1.0f, +1.0f), math::XMFLOAT3(-1.0f, +1.0f, +1.0f),
            math::XMFLOAT3(+1.0f, +1.0f, +1.0f), math::XMFLOAT3(+1.0f, -1.0f, +1.0f),
        };

        m_cubemap_position_buffer_index = Renderer::instance().create_buffer<math::XMFLOAT3>(
            rhi::BufferCreationDesc{
                .usage = rhi::BufferUsage::StructuredBuffer,
                .name = L"Cubemap position buffer",
            },
            positions);

        // Create index array
        constexpr auto indices = std::array<uint16_t, 36>{
            0, 1, 2, 0, 2, 3, 4, 6, 5, 4, 7, 6, 4, 5, 1, 4, 1, 0, 3, 2, 6, 3, 6, 7, 1, 5, 6, 1, 6, 2, 4, 0, 3, 4, 3, 7,
        };

        m_cubemap_index_buffer_index = Renderer::instance().create_buffer<uint16_t>(
            rhi::BufferCreationDesc{
                .usage = rhi::BufferUsage::StructuredBuffer,
                .name = L"Cubemap index buffer",
            },
            indices);
    }

    CubeMapRenderpass::~CubeMapRenderpass()
    {
        core::Log::instance().info("Destroyed cube map render pass");
    }

    void CubeMapRenderpass::render(rhi::DescriptorHeap &cbv_srv_uav_descriptor_heap, rhi::CommandList &command_list,
                                   const uint32_t scene_buffer_cbv_index, const uint32_t texture_srv_index) const
    {
        // Render cube map.

        // Set pipeline and root signature state.
        command_list.set_bindless_graphics_root_signature();
        command_list.set_pipeline_state(renderer::Renderer::instance().get_pipeline_at_index(m_cubemap_pipeline_index));

        command_list.set_descriptor_heaps(std::array{&cbv_srv_uav_descriptor_heap});

        const auto cubemap_render_resources = CubeMapRenderResources{
            .texture_srv_index = texture_srv_index,
            .position_buffer_srv_index =
                Renderer::instance().get_buffer_at_index(m_cubemap_position_buffer_index).srv_index,
            .scene_buffer_cbv_index = scene_buffer_cbv_index,
        };

        command_list.set_graphics_32_bit_root_constants(reinterpret_cast<const std::byte *>(&cubemap_render_resources));

        command_list.set_index_buffer(Renderer::instance().get_buffer_at_index(m_cubemap_index_buffer_index));
        command_list.draw_indexed_instanced(36, 1u);
    }
} // namespace serenity::renderer::renderpass