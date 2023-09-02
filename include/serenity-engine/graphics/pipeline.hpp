#pragma once

#include "d3d_utils.hpp"

#include "shader_compiler.hpp"

namespace serenity::graphics
{
    struct PipelineCreationDesc
    {
        Shader vertex_shader{};
        Shader pixel_shader{};

        std::wstring name{};
    };

    // Abstraction for the pipeline state object, which represents the state of set shaders and the other fixed function
    // state objects.
    class Pipeline
    {
      public:
        Pipeline() = default;
        Pipeline(const comptr<ID3D12Device> &device, const PipelineCreationDesc &pipeline_creation_desc);
        ~Pipeline() = default;

        comptr<ID3D12PipelineState> get_pipeline_state() const
        {
            return m_pipeline_state;
        }

      private:
        comptr<ID3D12PipelineState> m_pipeline_state{};
    };
} // namespace serenity::graphics