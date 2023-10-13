#pragma once

#include "serenity-engine/core/singleton_instance.hpp"

#include "renderpass/atmosphere_renderpass.hpp"
#include "renderpass/cube_map_renderpass.hpp"
#include "renderpass/post_processing_renderpass.hpp"
#include "renderpass/shading_renderpass.hpp"

#include "serenity-engine/renderer/rhi/device.hpp"
#include "serenity-engine/renderer/shader_compiler.hpp"
#include "serenity-engine/window/window.hpp"

namespace serenity::renderer
{
    // The renderer is the primary interface the application / game will use to handle all rendering related task
    // (including creation of GPU resources such as buffers, textures, etc, and all render passes). This has been done
    // so as to have minimum connect between the backend graphics api and the application (note that while the engine
    // will only support D3D12 for now, it is done as a learning exercise).

    class Renderer : public core::SingletonInstance<Renderer>
    {
      public:
        explicit Renderer(window::Window &window);
        ~Renderer();

        rhi::Device &get_device() const
        {
            return *(m_device.get());
        }

        Uint2 get_render_area_dimensions() const
        {
            return window_ref.get_dimensions();
        }

        rhi::Buffer &get_buffer_at_index(const uint32_t index)
        {
            return m_allocated_buffers.at(index);
        }

        rhi::Texture &get_texture_at_index(const uint32_t index)
        {
            return m_allocated_textures.at(index);
        }

        rhi::Pipeline &get_pipeline_at_index(const uint32_t index)
        {
            return m_pipelines.at(index);
        }

        interop::AtmosphereRenderPassBuffer &get_atmosphere_renderpass_buffer()
        {
            return m_atmosphere_renderpass->get_atmosphere_renderpass_buffer_data();
        }

        interop::PostProcessBuffer &get_post_process_renderpass_buffer()
        {
            return m_post_processing_renderpass->get_post_process_buffer();
        }

        // Create GPU buffer and return index to the created buffer.
        template <typename T>
        uint32_t create_buffer(const rhi::BufferCreationDesc &buffer_creation_desc, const std::span<const T> data = {})
        {
            const auto index = m_allocated_buffers.size();
            m_allocated_buffers.emplace_back(m_device->create_buffer(buffer_creation_desc, data));

            return index;
        }

        // Create GPU texture and return index to the created texture.
        uint32_t create_texture(const rhi::TextureCreationDesc &texture_creation_desc, const std::byte *data = nullptr)
        {
            const auto index = m_allocated_textures.size();
            m_allocated_textures.emplace_back(m_device->create_texture(texture_creation_desc, data));

            return index;
        }

        // Create a pipeline and return index to pipeline.
        uint32_t create_pipeline(const rhi::PipelineCreationDesc &pipeline_creation_desc)
        {
            auto pipeline = m_device->create_pipeline(pipeline_creation_desc);
            const auto pipeline_name = wstring_to_string(pipeline_creation_desc.name);

            const auto index = m_pipelines.size();
            pipeline.index = index;

            m_pipelines.emplace_back(pipeline);

            return index;
        }

        // Pipelines are reloaded at the end of the frame.
        void schedule_pipeline_for_reload(const uint32_t index)
        {
            m_pipeline_reload_buffer.push_back(index);
        }

        std::vector<rhi::Pipeline> &get_pipelines()
        {
            return m_pipelines;
        }

        // Render the current scene (uses the SceneManager to fetch this information).
        void render();

        void update_renderpasses(const uint32_t frame_count);

      private:
        // Create resources for rendering.
        void create_resources();

        // Create the renderpasses.
        void create_renderpasses();

        // Note : reloading of pipelines can ONLY occur at the end of the current frame.
        void reload_pipelines();

      private:
        Renderer(const Renderer &other) = delete;
        Renderer &operator=(const Renderer &other) = delete;

        Renderer(Renderer &&other) = delete;
        Renderer &operator=(Renderer &&other) = delete;

      private:
        std::unique_ptr<rhi::Device> m_device{};
        std::unique_ptr<ShaderCompiler> m_shader_compiler{};

        // The renderer will hold a vector of buffers and textures, so that the callers (application / game) will just
        // receive a index and be unaware of the internals of the buffer / texture. This might not be the most scalable
        // solution for the current state of engine is a good solution for convenience.
        std::vector<rhi::Buffer> m_allocated_buffers{};
        std::vector<rhi::Texture> m_allocated_textures{};

        // Renderer will hold a vector of pipelines so pipelines can be reloaded at runtime.
        // The application / game will only have a index to the pipeline in the vector. This might not be the most
        // scalable solution for the current state of engine is a good solution for convenience.
        std::vector<rhi::Pipeline> m_pipelines{};

        // Since reloading of pipelines can only occur at the end of each frame, we hold pipeline reload requests in
        // this vector (for the current frame).
        std::vector<uint32_t> m_pipeline_reload_buffer{};

        // Renderpasses.
        std::unique_ptr<renderpass::AtmosphereRenderpass> m_atmosphere_renderpass{};
        std::unique_ptr<renderpass::CubeMapRenderpass> m_cube_map_renderpass{};
        std::unique_ptr<renderpass::ShadingRenderpass> m_shading_renderpass{};
        std::unique_ptr<renderpass::PostProcessingRenderpass> m_post_processing_renderpass{};

        // Textures to render into.
        rhi::Texture m_depth_texture{};
        rhi::Texture m_render_texture{};

        window::Window &window_ref;
    };
} // namespace serenity::renderer