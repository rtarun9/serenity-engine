#pragma once

#include "serenity-engine/renderer/rhi/command_list.hpp"
#include "serenity-engine/renderer/rhi/pipeline.hpp"

namespace serenity::renderer::renderpass
{
    // Renderpass that takes in a render_texture and applies post processing to it.
    class PostProcessingRenderpass
    {
      public:
        explicit PostProcessingRenderpass();
        ~PostProcessingRenderpass();

        void render(rhi::CommandList &command_list, const uint32_t render_texture_srv_index) const;

      private:
        PostProcessingRenderpass(const PostProcessingRenderpass &other) = delete;
        PostProcessingRenderpass &operator=(const PostProcessingRenderpass &other) = delete;

        PostProcessingRenderpass(PostProcessingRenderpass &&other) = delete;
        PostProcessingRenderpass &operator=(PostProcessingRenderpass &&other) = delete;

      private:
        uint32_t m_fullscreen_triangle_index_buffer_index{};

        uint32_t m_post_process_pipeline_index{};
    };
} // namespace serenity::renderer::renderpass