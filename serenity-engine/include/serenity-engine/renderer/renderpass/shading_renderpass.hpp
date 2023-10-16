#pragma once

#include "serenity-engine/renderer/rhi/command_list.hpp"
#include "serenity-engine/renderer/rhi/pipeline.hpp"

#include "serenity-engine/renderer/rhi/descriptor_heap.hpp"

namespace serenity::renderer::renderpass
{
    // Renderpass that abstracts away the actual object shading & rendering (of the current scene).
    // For simplicity all objects in the scene are rendered using the same shading model,
    // hence why there are no specific renderpasses for each shading method (PBR, Phong, etc).
    class ShadingRenderpass
    {
      public:
        explicit ShadingRenderpass();
        ~ShadingRenderpass();

        void render(rhi::CommandList &command_list, const uint32_t scene_buffer_cbv_index,
                    const uint32_t atmosphere_texture_srv_index) const;

      private:
        ShadingRenderpass(const ShadingRenderpass &other) = delete;
        ShadingRenderpass &operator=(const ShadingRenderpass &other) = delete;

        ShadingRenderpass(ShadingRenderpass &&other) = delete;
        ShadingRenderpass &operator=(ShadingRenderpass &&other) = delete;

      private:
        uint32_t m_shading_pipeline_index{};
    };
} // namespace serenity::renderer::renderpass