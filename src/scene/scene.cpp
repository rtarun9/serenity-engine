#include "serenity-engine/scene/scene.hpp"
#include "serenity-engine/asset/model_loader.hpp"

#include "serenity-engine/renderer/rhi/device.hpp"

namespace serenity::scene
{
    Scene::Scene(const std::string_view scene_name) : m_scene_name(scene_name)
    {
        core::Log::instance().info(std::format("Created scene {}", scene_name));
    }

    void Scene::add_model(const std::string_view model_path, const math::XMMATRIX transform,
                          const std::string_view model_name)
    {
        auto model = Model{};
        model.transform_component.transform_buffer =
            renderer::rhi::Device::instance().create_buffer<TransformBuffer>(renderer::rhi::BufferCreationDesc{
                .usage = renderer::rhi::BufferUsage::ConstantBuffer,
                .name = string_to_wstring(model_path) + L" Transform Buffer",
            });

        model.model_name = model_name;

        for (const auto model_data = asset::ModelLoader::load_model(model_path, transform);
             const auto &mesh_data : model_data.mesh_data)
        {
            auto mesh = Mesh{};
            const auto model_path_wstring = string_to_wstring(model_path);

            mesh.position_buffer = renderer::rhi::Device::instance().create_buffer<math::XMFLOAT3>(
                renderer::rhi::BufferCreationDesc{
                    .usage = renderer::rhi::BufferUsage::StructuredBuffer,
                    .name = model_path_wstring + L"Position Buffer",
                },
                mesh_data.positions);

            mesh.normal_buffer = renderer::rhi::Device::instance().create_buffer<math::XMFLOAT3>(
                renderer::rhi::BufferCreationDesc{.usage = renderer::rhi::BufferUsage::StructuredBuffer,
                                                  .name = model_path_wstring + L" Normal Buffer"},
                mesh_data.normals);

            mesh.texture_coords_buffer = renderer::rhi::Device::instance().create_buffer<math::XMFLOAT2>(
                renderer::rhi::BufferCreationDesc{.usage = renderer::rhi::BufferUsage::StructuredBuffer,
                                                  .name = model_path_wstring + L" Texture Coords Buffer"},
                mesh_data.texture_coords);

            mesh.index_buffer = renderer::rhi::Device::instance().create_buffer<uint16_t>(
                renderer::rhi::BufferCreationDesc{
                    .usage = renderer::rhi::BufferUsage::IndexBuffer,
                    .name = model_path_wstring + L" Index Buffer",
                },
                mesh_data.indices);

            mesh.indices = mesh_data.indices.size();

            model.meshes.emplace_back(std::move(mesh));
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