#pragma once

#include "serenity-engine/core/singleton_instance.hpp"

#include "serenity-engine/renderer/rhi/device.hpp"
#include "serenity-engine/renderer/shader_compiler.hpp"
#include "serenity-engine/window/window.hpp"

namespace serenity::renderer
{
    // The renderer is the primary interface the application / game will use to handle all rendering related task
    // (including creation of GPU resources such as buffers, textures, etc). This has been done so as to have minimum
    // connect between the backend graphics api and the application (note that while the engine will only support D3D12
    // for now, it is done as a learning exercise).

    class Renderer : public core::SingletonInstance<Renderer>
    {
      public:
        explicit Renderer(window::Window &window);
        ~Renderer();

        void render();

      private:
        // Create resources for rendering.
        void create_resources();

        // Create the renderpasses.
        void create_renderpasses();

      private:
        Renderer(const Renderer &other) = delete;
        Renderer &operator=(const Renderer &other) = delete;

        Renderer(Renderer &&other) = delete;
        Renderer &operator=(Renderer &&other) = delete;

      public:
        std::unique_ptr<rhi::Device> m_device{};
        std::unique_ptr<ShaderCompiler> m_shader_compiler{};

        // Renderpasses

        // Resources for rendering.
        rhi::Pipeline m_pipeline{};

        rhi::Texture m_depth_texture{};
        rhi::Texture m_albedo_texture{};

        window::Window &window_ref;
    };
} // namespace serenity::renderer