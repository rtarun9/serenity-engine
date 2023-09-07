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

        Shader vertex_shader{};
        Shader pixel_shader{};

        DXGI_FORMAT dsv_format{DXGI_FORMAT_UNKNOWN};

        std::wstring name{};
    };

    // Abstraction for the pipeline state object, which represents the state of set shaders and the other fixed function
    // state objects.
    // The same constructor is used to determine if a graphics or compute pipeline is to be created (according to the
    // pipeline_variant field in the pipeline creation desc).
    // note(rtarun9) : Currently this classes move constructor and assignment operator will be used quite a lot,
    // evaluate if this is ok.
    class Pipeline
    {
      public:
        Pipeline() = default;
        Pipeline(const comptr<ID3D12Device> &device, const PipelineCreationDesc &pipeline_creation_desc);
        ~Pipeline() = default;

        Pipeline(const Pipeline &other) = delete;
        Pipeline &operator=(const Pipeline &other) = delete;

        Pipeline(Pipeline &&other) noexcept
        {
            if (this != &other)
            {
                this->m_pipeline_state = other.m_pipeline_state;
                other.m_pipeline_state.Reset();
            }
        }

        Pipeline &operator=(Pipeline &&other) noexcept
        {
            if (this != &other)
            {
                this->m_pipeline_state = other.m_pipeline_state;
                other.m_pipeline_state.Reset();
            }

            return *this;
        }

        comptr<ID3D12PipelineState> get_pipeline_state() const
        {
            return m_pipeline_state;
        }

      private:
        comptr<ID3D12PipelineState> m_pipeline_state{};
    };
} // namespace serenity::renderer::rhi