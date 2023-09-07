#include "serenity-engine/scene/scene.hpp"
#include "serenity-engine/asset/model_loader.hpp"

#include "serenity-engine/renderer/renderer.hpp"

namespace serenity::scene
{
    Scene::Scene(const std::string_view scene_name) : m_scene_name(scene_name)
    {
        core::Log::instance().info(std::format("Created scene {}", scene_name));
    }

    void Scene::add_model(const std::string_view model_path, const std::string_view model_name,
                          const math::XMFLOAT3 scale)
    {
        auto model = Model{};

        // Create the transform component for the model.
        model.transform_component.transform_buffer_index =
            renderer::Renderer::instance().create_buffer<TransformBuffer>(renderer::rhi::BufferCreationDesc{
                .usage = renderer::rhi::BufferUsage::ConstantBuffer,
                .name = string_to_wstring(model_path) + L" Transform Buffer",
            });

        model.transform_component.scale = scale;
        model.model_name = model_name;

        // Load the model data (meshes + materials) and create GPU buffers / textures for them.
        const auto model_data = asset::ModelLoader::load_model(model_path);

        for (const auto &mesh_data : model_data.mesh_data)
        {
            auto mesh = Mesh{};
            const auto model_path_wstring = string_to_wstring(model_path);

            mesh.position_buffer_index = renderer::Renderer::instance().create_buffer<math::XMFLOAT3>(
                renderer::rhi::BufferCreationDesc{
                    .usage = renderer::rhi::BufferUsage::StructuredBuffer,
                    .name = model_path_wstring + L"Position Buffer",
                },
                mesh_data.positions);

            mesh.normal_buffer_index = renderer::Renderer::instance().create_buffer<math::XMFLOAT3>(
                renderer::rhi::BufferCreationDesc{.usage = renderer::rhi::BufferUsage::StructuredBuffer,
                                                  .name = model_path_wstring + L" Normal Buffer"},
                mesh_data.normals);

            mesh.texture_coords_buffer_index = renderer::Renderer::instance().create_buffer<math::XMFLOAT2>(
                renderer::rhi::BufferCreationDesc{.usage = renderer::rhi::BufferUsage::StructuredBuffer,
                                                  .name = model_path_wstring + L" Texture Coords Buffer"},
                mesh_data.texture_coords);

            mesh.index_buffer_index = renderer::Renderer::instance().create_buffer<uint16_t>(
                renderer::rhi::BufferCreationDesc{
                    .usage = renderer::rhi::BufferUsage::IndexBuffer,
                    .name = model_path_wstring + L" Index Buffer",
                },
                mesh_data.indices);

            mesh.material_index = mesh_data.material_index;

            mesh.indices = mesh_data.indices.size();

            model.meshes.emplace_back(std::move(mesh));
        }

        for (const auto &material_data : model_data.material_data)
        {
            auto material = Material{};

            const auto base_color_texture_data = std::get<std::vector<uint8_t>>(material_data.base_color_texture.data);

            material.base_color_texture_index = renderer::Renderer::instance().create_texture(
                renderer::rhi::TextureCreationDesc{
                    .usage = renderer::rhi::TextureUsage::ShaderResourceTexture,
                    .format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .bytes_per_pixel = 4u,
                    .dimension = material_data.base_color_texture.dimension,
                },
                reinterpret_cast<const std::byte *>(base_color_texture_data.data()));

            model.materials.emplace_back(std::move(material));
        }

        m_models.emplace_back(model);
    }

    void Scene::update(const math::XMMATRIX projection_matrix)
    {
        for (auto &model : m_models)
        {
            model.transform_component.update(m_camera.get_view_matrix() * projection_matrix);
        }
    }
} // namespace serenity::scene