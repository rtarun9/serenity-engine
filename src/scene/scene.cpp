#include "serenity-engine/scene/scene.hpp"
#include "serenity-engine/asset/model_loader.hpp"

#include "serenity-engine/renderer/renderer.hpp"

namespace serenity::scene
{
    Scene::Scene(const std::string_view scene_name) : m_scene_name(scene_name)
    {
        // Create the scene buffer.
        m_scene_buffer_index = renderer::Renderer::instance().create_buffer<SceneBuffer>(
            renderer::rhi::BufferCreationDesc{.usage = renderer::rhi::BufferUsage::ConstantBuffer,
                                              .name = string_to_wstring(scene_name) + L" Scene Buffer"});

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

            const auto &base_color_texture_data = std::get<std::vector<uint8_t>>(material_data.base_color_texture.data);

            material.material_data.albedo_texture_srv_index = INVALID_INDEX_U32;

            if (material_data.base_color_texture.dimension.x != 0 && material_data.base_color_texture.dimension.y != 0)
            {
                const auto albedo_texture_index = renderer::Renderer::instance().create_texture(
                    renderer::rhi::TextureCreationDesc{
                        .usage = renderer::rhi::TextureUsage::ShaderResourceTexture,
                        .format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                        .bytes_per_pixel = 4u,
                        .dimension = material_data.base_color_texture.dimension,
                        .name = string_to_wstring(model_name) + L" albedo texture",
                    },
                    reinterpret_cast<const std::byte *>(base_color_texture_data.data()));

                material.material_data.albedo_texture_srv_index =
                    renderer::Renderer::instance().get_texture_at_index(albedo_texture_index).srv_index;
            }

            material.material_data.base_color = material_data.base_color;

            // Create the material buffer.
            material.material_buffer_index = renderer::Renderer::instance().create_buffer<MaterialBuffer>(
                renderer::rhi::BufferCreationDesc{
                    .usage = renderer::rhi::BufferUsage::ConstantBuffer,
                    .name = string_to_wstring(model_name) + L" material buffer",
                },
                std::array{material.material_data});

            model.materials.emplace_back(std::move(material));
        }

        m_models.emplace_back(model);
    }

    void Scene::update(const math::XMMATRIX projection_matrix, const float delta_time, const core::Input &input)
    {
        m_camera.update(delta_time, input);

        m_lights.update(m_scene_buffer.view_matrix);

        // Update scene buffer.
        m_scene_buffer.view_projection_matrix = m_camera.get_view_matrix() * projection_matrix;
        m_scene_buffer.inverse_projection_matrix = math::XMMatrixInverse(nullptr, projection_matrix);
        m_scene_buffer.inverse_view_projection_matrix =
            math::XMMatrixInverse(nullptr, m_scene_buffer.view_projection_matrix);
        m_scene_buffer.view_matrix = m_camera.get_view_matrix();
        m_scene_buffer.inverse_view_matrix = math::XMMatrixInverse(nullptr, m_camera.get_view_matrix());

        m_scene_buffer.camera_position =
            math::XMFLOAT3{m_camera.m_camera_position.x, m_camera.m_camera_position.y, m_camera.m_camera_position.z};

        renderer::Renderer::instance()
            .get_buffer_at_index(m_scene_buffer_index)
            .update(reinterpret_cast<const std::byte *>(&m_scene_buffer), sizeof(SceneBuffer));

        for (auto &model : m_models)
        {
            model.transform_component.update();
        }
    }

    void Scene::add_light(const Light &light)
    {
        m_lights.add_light(light);
    }
} // namespace serenity::scene