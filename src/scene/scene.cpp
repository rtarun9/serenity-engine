#include "serenity-engine/scene/scene.hpp"
#include "serenity-engine/asset/model_loader.hpp"

#include "serenity-engine/graphics/device.hpp"

namespace serenity::scene
{
    Scene::Scene(const std::string_view scene_name) : m_scene_name(scene_name)
    {
        core::Log::instance().info(std::format("Created scene {}", scene_name));
    }

    void Scene::add_model(const std::string_view model_path)
    {
        auto model = Model{};
        for (const auto model_data = asset::ModelLoader::load_model(model_path);
             const auto &mesh_data : model_data.cpu_meshes)
        {
            auto mesh = Mesh{};
            const auto model_path_wstring = string_to_wstring(model_path);

            mesh.position_buffer = graphics::Device::instance().create_buffer<math::XMFLOAT3>(
                graphics::BufferCreationDesc{
                    .usage = graphics::BufferUsage::StructuredBuffer,
                    .name = model_path_wstring + L"Position Buffer",
                },
                mesh_data.positions);

            mesh.normal_buffer = graphics::Device::instance().create_buffer<math::XMFLOAT3>(
                graphics::BufferCreationDesc{.usage = graphics::BufferUsage::StructuredBuffer,
                                             .name = model_path_wstring + L" Normal Buffer"},
                mesh_data.normals);

            mesh.texture_coords_buffer = graphics::Device::instance().create_buffer<math::XMFLOAT2>(
                graphics::BufferCreationDesc{.usage = graphics::BufferUsage::StructuredBuffer,
                                             .name = model_path_wstring + L" Texture Coords Buffer"},
                mesh_data.texture_coords);

            mesh.index_buffer = graphics::Device::instance().create_buffer<uint16_t>(
                graphics::BufferCreationDesc{
                    .usage = graphics::BufferUsage::IndexBuffer,
                    .name = model_path_wstring + L" Index Buffer",
                },
                mesh_data.indices);

            mesh.indices = mesh_data.indices.size();

            model.meshes.emplace_back(std::move(mesh));
        }

        m_models.emplace_back(model);
    }
} // namespace serenity::scene