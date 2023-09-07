#include "serenity-engine/renderer/rhi/pipeline.hpp"

#include "serenity-engine/renderer/rhi/root_signature.hpp"

namespace serenity::renderer::rhi
{
    Pipeline::Pipeline(const comptr<ID3D12Device> &device, const PipelineCreationDesc &pipeline_creation_desc)
    {
        core::Log::instance().info(
            std::format("Created pipeline object with name {}", wstring_to_string(pipeline_creation_desc.name)));

        // Create pipeline state object.
        if (pipeline_creation_desc.pipeline_variant == PipelineVariant::Graphics)
        {
            const auto depth_enable = pipeline_creation_desc.dsv_format != DXGI_FORMAT_UNKNOWN;

            const auto graphics_pipeline_state_desc = D3D12_GRAPHICS_PIPELINE_STATE_DESC{

                .pRootSignature = RootSignature::instance().get_root_signature().Get(),
                .VS =
                    {

                        .pShaderBytecode = pipeline_creation_desc.vertex_shader.blob->GetBufferPointer(),
                        .BytecodeLength = pipeline_creation_desc.vertex_shader.blob->GetBufferSize(),
                    },
                .PS =
                    {

                        .pShaderBytecode = pipeline_creation_desc.pixel_shader.blob->GetBufferPointer(),
                        .BytecodeLength = pipeline_creation_desc.pixel_shader.blob->GetBufferSize(),
                    },
                .BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
                .SampleMask = D3D12_DEFAULT_SAMPLE_MASK,
                .RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
                .DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(
                    depth_enable, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS_EQUAL, false, 0u, 0u,
                    D3D12_STENCIL_OP_ZERO, D3D12_STENCIL_OP_ZERO, D3D12_STENCIL_OP_ZERO,
                    D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STENCIL_OP_ZERO, D3D12_STENCIL_OP_ZERO,
                    D3D12_STENCIL_OP_ZERO, D3D12_COMPARISON_FUNC_EQUAL),
                .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
                .NumRenderTargets = 1u,
                .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM},
                .DSVFormat = pipeline_creation_desc.dsv_format,
                .SampleDesc = {1u, 0u},
                .NodeMask = 0u,
                .Flags = D3D12_PIPELINE_STATE_FLAG_NONE,
            };

            throw_if_failed(
                device->CreateGraphicsPipelineState(&graphics_pipeline_state_desc, IID_PPV_ARGS(&m_pipeline_state)));
        }
        else if (pipeline_creation_desc.pipeline_variant == PipelineVariant::Compute)
        {
        }
    }
} // namespace serenity::renderer::rhi