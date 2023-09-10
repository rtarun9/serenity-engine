#include "serenity-engine/renderer/renderpass/atmosphere_renderpass.hpp"

#include "serenity-engine/renderer/renderer.hpp"

namespace serenity::renderer::renderpass
{
    AtmosphereRenderpass::AtmosphereRenderpass()
    {
        core::Log::instance().info("Created atmosphere render pass");

        // Create buffers.
        m_atmosphere_buffer_index =
            Renderer::instance().create_buffer<AtmosphereRenderPassBuffer>(rhi::BufferCreationDesc{
                .usage = rhi::BufferUsage::ConstantBuffer,
                .name = L"Atmosphere Render Pass buffer",
            });
    }

    AtmosphereRenderpass::~AtmosphereRenderpass()
    {
        core::Log::instance().info("Destroyed atmosphere render pass");
    }

    void AtmosphereRenderpass::update()
    {
        Renderer::instance()
            .get_buffer_at_index(m_atmosphere_buffer_index)
            .update(reinterpret_cast<const std::byte *>(&m_atmosphere_buffer_data), sizeof(AtmosphereRenderPassBuffer));
    }
} // namespace serenity::renderer::renderpass