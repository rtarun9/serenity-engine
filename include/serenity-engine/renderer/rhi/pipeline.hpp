#pragma once

#include "d3d_utils.hpp"

#include "serenity-engine/renderer/shader.hpp"

namespace serenity::renderer::rhi
{
    enum class PipelineVariant
    {
        Graphics,
        Compute,
    };

    struct PipelineCreationDesc
    {
        PipelineVariant pipeline_variant{};

        std::optional<ShaderCreationDesc> vertex_shader_creation_desc{};
        std::optional<ShaderCreationDesc> pixel_shader_creation_desc{};
        std::optional<ShaderCreationDesc> compute_shader_creation_desc{};

        D3D12_CULL_MODE cull_mode{D3D12_CULL_MODE_BACK};

        std::vector<DXGI_FORMAT> rtv_formats{};
        DXGI_FORMAT dsv_format{DXGI_FORMAT_UNKNOWN};

        std::wstring name{};
    };

    // Abstraction for the pipeline state object, which represents the state of set shaders and the other fixed function
    // state objects.
    // Holds the pipeline creation desc since for shader hot-reloading that information would be required.
    struct Pipeline
    {
        comptr<ID3D12PipelineState> pipeline_state{};

        PipelineCreationDesc pipeline_creation_desc{};

        // Index into the renderer's vector of pipelines.
        uint32_t index{};
    };
} // namespace serenity::renderer::rhi