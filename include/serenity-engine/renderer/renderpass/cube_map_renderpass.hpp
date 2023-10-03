#pragma once

#include "serenity-engine/renderer/rhi/command_list.hpp"
#include "serenity-engine/renderer/rhi/pipeline.hpp"

#include "serenity-engine/renderer/rhi/descriptor_heap.hpp"

namespace serenity::renderer::renderpass
{
    // Renderpass that takes in a texture map srv index as input and renders a cube with given texture.
    class CubeMapRenderpass
    {
      public:
        explicit CubeMapRenderpass();
        ~CubeMapRenderpass();

        void render(rhi::CommandList &command_list, const uint32_t scene_buffer_cbv_index,
                    const uint32_t texture_srv_index) const;

      private:
        CubeMapRenderpass(const CubeMapRenderpass &other) = delete;
        CubeMapRenderpass &operator=(const CubeMapRenderpass &other) = delete;

        CubeMapRenderpass(CubeMapRenderpass &&other) = delete;
        CubeMapRenderpass &operator=(CubeMapRenderpass &&other) = delete;

      private:
        uint32_t m_cubemap_position_buffer_index{};
        uint32_t m_cubemap_index_buffer_index{};

        uint32_t m_cubemap_pipeline_index{};
    };
} // namespace serenity::renderer::renderpass