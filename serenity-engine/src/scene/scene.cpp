#include "serenity-engine/scene/scene.hpp"

#include "serenity-engine/asset/model_loader.hpp"
#include "serenity-engine/core/file_system.hpp"
#include "serenity-engine/renderer/renderer.hpp"

namespace serenity::scene
{
    Scene::Scene(const std::string_view scene_name, const std::string_view scene_init_script_path)
    {
        m_scene_name = scene_name;

        m_game_objects.reserve(Scene::MAX_GAME_OBJECTS);
        m_scene_resources.game_object_buffers.resize(Scene::MAX_GAME_OBJECTS);

        const auto script_path = core::FileSystem::instance().get_absolute_path(scene_init_script_path);

        m_scene_init_script_index = scripting::ScriptManager::instance().create_script(scripting::Script{
            .script_name = std::string(scene_name) + " init script",
            .script_path = script_path,
        });

        load_scene_from_script();

        core::Log::instance().info(std::format("Created scene {}", scene_name));
    }

    void Scene::reload()
    {
        m_scene_resources.game_object_buffers.clear();
        m_scene_resources.indices.clear();
        m_scene_resources.material_buffers.clear();
        m_scene_resources.mesh_buffers.clear();
        m_scene_resources.normals.clear();
        m_scene_resources.positions.clear();
        m_scene_resources.texture_coords.clear();

        m_game_objects.clear();

        m_game_objects.reserve(Scene::MAX_GAME_OBJECTS);
        m_scene_resources.game_object_buffers.resize(Scene::MAX_GAME_OBJECTS);

        load_scene_from_script();
    }

    void Scene::update(const math::XMMATRIX projection_matrix, const float delta_time, const uint32_t frame_count,
                       const core::Input &input)
    {
        m_camera.update(delta_time, input);

        m_lights.update(m_scene_resources.scene_buffer.view_matrix);

        // Update scene buffer.
        m_scene_resources.scene_buffer.view_projection_matrix = m_camera.get_view_matrix() * projection_matrix;
        m_scene_resources.scene_buffer.inverse_projection_matrix = math::XMMatrixInverse(nullptr, projection_matrix);
        m_scene_resources.scene_buffer.inverse_view_projection_matrix =
            math::XMMatrixInverse(nullptr, m_scene_resources.scene_buffer.view_projection_matrix);
        m_scene_resources.scene_buffer.view_matrix = m_camera.get_view_matrix();
        m_scene_resources.scene_buffer.inverse_view_matrix = math::XMMatrixInverse(nullptr, m_camera.get_view_matrix());

        m_scene_resources.scene_buffer.camera_position =
            math::XMFLOAT3{m_camera.m_camera_position.x, m_camera.m_camera_position.y, m_camera.m_camera_position.z};

        renderer::Renderer::instance()
            .get_buffer_at_index(m_scene_resources.scene_buffer_index)
            .update(reinterpret_cast<const std::byte *>(&m_scene_resources.scene_buffer), sizeof(interop::SceneBuffer));

        m_scene_resources.game_object_buffers.clear();

        for (auto &[name, game_object] : m_game_objects)
        {
            game_object.update(delta_time, frame_count);

            const auto game_object_data = interop::GameObjectBuffer{
                .transform_buffer = game_object.transform_component.transform_buffer_data,
            };

            m_scene_resources.game_object_buffers.push_back(game_object_data);
        }

        renderer::Renderer::instance()
            .get_buffer_at_index(m_scene_resources.game_object_buffer_index)
            .update(reinterpret_cast<const std::byte *>(m_scene_resources.game_object_buffers.data()),
                    sizeof(interop::GameObjectBuffer) * m_scene_resources.game_object_buffers.size());

        renderer::Renderer::instance()
            .get_buffer_at_index(m_scene_resources.materal_buffer_index)
            .update(reinterpret_cast<const std::byte *>(m_scene_resources.material_buffers.data()),
                    sizeof(interop::MaterialBuffer) * m_scene_resources.material_buffers.size());
    }

    void Scene::load_scene_from_script()
    {
        scripting::ScriptManager::instance().execute_script(m_scene_init_script_index.value());

        sol::table game_objects = scripting::ScriptManager::instance().get_state()["game_objects"];

        for (auto &key_value_pair : game_objects)
        {
            const auto game_object_name = key_value_pair.first.as<std::string>();
            const auto value = key_value_pair.second.as<sol::table>();

            const std::string model_path = value["model_path"];

            const math::XMFLOAT3 scale = {
                value["scale"]["x"],
                value["scale"]["y"],
                value["scale"]["z"],
            };
            const math::XMFLOAT3 rotation = {
                value["rotation"]["x"],
                value["rotation"]["y"],
                value["rotation"]["z"],
            };
            const math::XMFLOAT3 translation = {
                value["translation"]["x"],
                value["translation"]["y"],
                value["translation"]["z"],
            };

            auto new_game_object = create_game_object(game_object_name, model_path);

            new_game_object.transform_component.scale = scale;
            new_game_object.transform_component.rotation = rotation;
            new_game_object.transform_component.translation = translation;

            const sol::table script = value["script"];
            if (!script.empty())
            {
                const std::string path = script["path"];
                new_game_object.script_index = scripting::ScriptManager::instance().create_script(scripting::Script{
                    .script_name = script["name"],
                    .script_path = core::FileSystem::instance().get_absolute_path(path),
                });
            }

            m_game_objects[game_object_name] = std::move(new_game_object);
        }

        create_scene_buffers();
    }

    void Scene::create_scene_buffers()
    {
        auto &scene_rsc = m_scene_resources;

        // Create the scene buffer.
        scene_rsc.scene_buffer_index =
            renderer::Renderer::instance().create_buffer<interop::SceneBuffer>(renderer::rhi::BufferCreationDesc{
                .usage = renderer::rhi::BufferUsage::ConstantBuffer,
                .name = string_to_wstring(m_scene_name) + L" Scene Buffer",
            });

        // Create scene positions buffer.
        scene_rsc.position_buffer_index = renderer::Renderer::instance().create_buffer<math::XMFLOAT3>(
            renderer::rhi::BufferCreationDesc{
                .usage = renderer::rhi::BufferUsage::StructuredBuffer,
                .name = string_to_wstring(m_scene_name) + L" Position Buffer",
            },
            scene_rsc.positions);

        // Create scene normal buffer.
        scene_rsc.normal_buffer_index = renderer::Renderer::instance().create_buffer<math::XMFLOAT3>(
            renderer::rhi::BufferCreationDesc{
                .usage = renderer::rhi::BufferUsage::StructuredBuffer,
                .name = string_to_wstring(m_scene_name) + L" Normal Buffer",
            },
            scene_rsc.normals);

        // Create scene teture coords buffer.
        scene_rsc.texture_coord_buffer_index = renderer::Renderer::instance().create_buffer<math::XMFLOAT2>(
            renderer::rhi::BufferCreationDesc{
                .usage = renderer::rhi::BufferUsage::StructuredBuffer,
                .name = string_to_wstring(m_scene_name) + L" Texture Coords Buffer",
            },
            scene_rsc.texture_coords);

        // Create scene indices buffer.
        scene_rsc.index_buffer_index = renderer::Renderer::instance().create_buffer<uint16_t>(
            renderer::rhi::BufferCreationDesc{
                .usage = renderer::rhi::BufferUsage::IndexBuffer,
                .name = string_to_wstring(m_scene_name) + L" Index Buffer",
            },
            scene_rsc.indices);

        // Create scene materials buffer.
        scene_rsc.materal_buffer_index = renderer::Renderer::instance().create_buffer<interop::MaterialBuffer>(
            renderer::rhi::BufferCreationDesc{
                .usage = renderer::rhi::BufferUsage::DynamicStructuredBuffer,
                .name = string_to_wstring(m_scene_name) + L" Material Buffer",
            },
            scene_rsc.material_buffers);

        // Create scene game object buffer.
        scene_rsc.game_object_buffer_index = renderer::Renderer::instance().create_buffer<interop::GameObjectBuffer>(
            renderer::rhi::BufferCreationDesc{
                .usage = renderer::rhi::BufferUsage::DynamicStructuredBuffer,
                .name = string_to_wstring(m_scene_name) + L" Game Object Buffer",
            },
            scene_rsc.game_object_buffers);

        // Create scene meshes buffer.
        scene_rsc.meshes_buffer_index = renderer::Renderer::instance().create_buffer<interop::MeshBuffer>(
            renderer::rhi::BufferCreationDesc{
                .usage = renderer::rhi::BufferUsage::StructuredBuffer,
                .name = string_to_wstring(m_scene_name) + L" Scene Meshes Buffer",
            },
            scene_rsc.mesh_buffers);
    }

    GameObject Scene::create_game_object(const std::string_view game_object_name,
                                         const std::string_view gltf_scene_path)
    {
        auto game_object = GameObject{};

        game_object.game_object_index = m_game_objects.size();
        game_object.game_object_name = game_object_name;

        // Load the model data (meshes + materials) and create GPU buffers / textures for them.
        const auto model_data = asset::ModelLoader::load_model(gltf_scene_path);

        game_object.mesh_count = model_data.mesh_data.size();
        game_object.material_count = model_data.material_data.size();

        game_object.mesh_buffer_offset = m_scene_resources.mesh_buffers.size();
        game_object.material_buffer_offset = m_scene_resources.material_buffers.size();

        auto meshes = std::vector<interop::MeshBuffer>{};
        for (const auto &mesh_data : model_data.mesh_data)
        {
            // Setup mesh_part.
            auto mesh_buffer = interop::MeshBuffer{
                .mesh_index = static_cast<uint32_t>(m_scene_resources.mesh_buffers.size() + meshes.size()),
                .game_object_index = game_object.game_object_index,

                .position_offset = static_cast<uint32_t>(m_scene_resources.positions.size()),
                .normal_offset = static_cast<uint32_t>(m_scene_resources.normals.size()),
                .texture_coord_offset = static_cast<uint32_t>(m_scene_resources.texture_coords.size()),

                .indices_offset = static_cast<uint32_t>(m_scene_resources.indices.size()),
                .indices_count = static_cast<uint32_t>(mesh_data.indices.size()),

                .mesh_local_transform_matrix = mesh_data.mesh_local_transform_matrix,
                .inverse_mesh_local_transform_matrix = mesh_data.inverse_mesh_local_transform_matrix,

                .material_index =
                    static_cast<uint32_t>(m_scene_resources.material_buffers.size() + mesh_data.material_index),
            };

            meshes.emplace_back(mesh_buffer);

            // Add data to the scene buffers.
            m_scene_resources.positions.insert(m_scene_resources.positions.end(), mesh_data.positions.begin(),
                                               mesh_data.positions.end());
            m_scene_resources.normals.insert(m_scene_resources.normals.end(), mesh_data.normals.begin(),
                                             mesh_data.normals.end());
            m_scene_resources.texture_coords.insert(m_scene_resources.texture_coords.end(),
                                                    mesh_data.texture_coords.begin(), mesh_data.texture_coords.end());

            m_scene_resources.indices.insert(m_scene_resources.indices.end(), mesh_data.indices.begin(),
                                             mesh_data.indices.end());
        }
        m_scene_resources.mesh_buffers.insert(m_scene_resources.mesh_buffers.end(), meshes.begin(), meshes.end());

        auto materials = std::vector<interop::MaterialBuffer>{};
        for (const auto &material_data : model_data.material_data)
        {
            auto material = interop::MaterialBuffer{};

            const auto &base_color_texture_data = std::get<std::vector<uint8_t>>(material_data.base_color_texture.data);

            material.albedo_texture_srv_index = INVALID_INDEX_U32;

            if (material_data.base_color_texture.dimension.x != 0 && material_data.base_color_texture.dimension.y != 0)
            {
                const auto albedo_texture_index = renderer::Renderer::instance().create_texture(
                    renderer::rhi::TextureCreationDesc{
                        .usage = renderer::rhi::TextureUsage::ShaderResourceTexture,
                        .format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                        .bytes_per_pixel = 4u,
                        .dimension = material_data.base_color_texture.dimension,
                        .name = string_to_wstring(game_object_name) + L" Albedo Texture Material " +
                                std::to_wstring(materials.size()),
                    },
                    reinterpret_cast<const std::byte *>(base_color_texture_data.data()));

                material.albedo_texture_srv_index =
                    renderer::Renderer::instance().get_texture_at_index(albedo_texture_index).srv_index;
            }

            material.base_color = material_data.base_color;
            material.metallic_roughness_factor = material_data.metallic_roughness_factor;

            materials.push_back(material);
        }

        m_scene_resources.material_buffers.insert(m_scene_resources.material_buffers.end(), materials.begin(),
                                                  materials.end());

        return game_object;
    }
} // namespace serenity::scene