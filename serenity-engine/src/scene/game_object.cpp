#include "serenity-engine/scene/game_object.hpp"

#include "serenity-engine/asset/model_loader.hpp"
namespace serenity::scene
{
    GameObject::GameObject(const std::string_view object_name, const std::string_view gltf_scene_path)
    {
        // Create the transform component for the game object.
        m_transform_component.transform_buffer_index =
            renderer::Renderer::instance().create_buffer<interop::TransformBuffer>(renderer::rhi::BufferCreationDesc{
                .usage = renderer::rhi::BufferUsage::ConstantBuffer,
                .name = string_to_wstring(gltf_scene_path) + L" Transform Buffer",
            });

        m_game_object_name = object_name;

        // Load the model data (meshes + materials) and create GPU buffers / textures for them.
        const auto model_data = asset::ModelLoader::load_model(gltf_scene_path);

        auto i = 0;

        for (const auto &mesh_data : model_data.mesh_data)
        {
            auto mesh = Mesh{};
            const auto model_path_wstring = string_to_wstring(gltf_scene_path);

            mesh.position_buffer_index = renderer::Renderer::instance().create_buffer<math::XMFLOAT3>(
                renderer::rhi::BufferCreationDesc{
                    .usage = renderer::rhi::BufferUsage::StructuredBuffer,
                    .name = model_path_wstring + L"Position Buffer Mesh " + std::to_wstring(i),
                },
                mesh_data.positions);

            mesh.normal_buffer_index = renderer::Renderer::instance().create_buffer<math::XMFLOAT3>(
                renderer::rhi::BufferCreationDesc{
                    .usage = renderer::rhi::BufferUsage::StructuredBuffer,
                    .name = model_path_wstring + L" Normal Buffer Mesh " + std::to_wstring(i),
                },
                mesh_data.normals);

            mesh.texture_coords_buffer_index = renderer::Renderer::instance().create_buffer<math::XMFLOAT2>(
                renderer::rhi::BufferCreationDesc{
                    .usage = renderer::rhi::BufferUsage::StructuredBuffer,
                    .name = model_path_wstring + L" Texture Coords Buffer Mesh " + std::to_wstring(i),
                },
                mesh_data.texture_coords);

            mesh.index_buffer_index = renderer::Renderer::instance().create_buffer<uint16_t>(
                renderer::rhi::BufferCreationDesc{
                    .usage = renderer::rhi::BufferUsage::IndexBuffer,
                    .name = model_path_wstring + L" Index Buffer Mesh " + std::to_wstring(i),
                },
                mesh_data.indices);

            mesh.material_index = mesh_data.material_index;

            mesh.indices = mesh_data.indices.size();

            m_meshes.emplace_back(std::move(mesh));

            ++i;
        }

        i = 0;

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
                        .name = string_to_wstring(object_name) + L" Albedo Texture Material " + std::to_wstring(i),
                    },
                    reinterpret_cast<const std::byte *>(base_color_texture_data.data()));

                material.material_data.albedo_texture_srv_index =
                    renderer::Renderer::instance().get_texture_at_index(albedo_texture_index).srv_index;
            }

            material.material_data.base_color = material_data.base_color;
            material.material_data.metallic_roughness_factor = material_data.metallic_roughness_factor;

            // Create the material buffer.
            material.material_buffer_index = renderer::Renderer::instance().create_buffer<interop::MaterialBuffer>(
                renderer::rhi::BufferCreationDesc{
                    .usage = renderer::rhi::BufferUsage::ConstantBuffer,
                    .name = string_to_wstring(m_game_object_name) + L" Material Buffer " + std::to_wstring(i),
                },
                std::array{material.material_data});

            m_materials.emplace_back(std::move(material));

            ++i;
        }
    }

    void GameObject::update(const float delta_time, const uint32_t frame_count)
    {
        if (m_script_index != INVALID_INDEX_U32)
        {
            scripting::ScriptManager::instance().execute_script(m_script_index);

            auto &transform = m_transform_component;

            std::tie(transform.scale, transform.rotation, transform.translation) =
                scripting::ScriptManager::instance().call_function()["update_transform"](
                    transform.scale, transform.rotation, transform.translation, delta_time, frame_count);
        }

        m_transform_component.update(delta_time, frame_count);

        for (auto &material_buffer : m_materials)
        {
            renderer::Renderer::instance()
                .get_buffer_at_index(material_buffer.material_buffer_index)
                .update(reinterpret_cast<const std::byte *>(&material_buffer.material_data),
                        sizeof(interop::MaterialBuffer));
        }
    }
} // namespace serenity::scene