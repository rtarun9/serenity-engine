#include "serenity-engine/scene/scene.hpp"
#include "serenity-engine/asset/model_loader.hpp"
#include "serenity-engine/core/file_system.hpp"
#include "serenity-engine/renderer/renderer.hpp"

namespace serenity::scene
{
    Scene::Scene(const std::string_view scene_name) : m_scene_name(scene_name)
    {
        // Create the scene buffer.
        m_scene_buffer_index = renderer::Renderer::instance().create_buffer<interop::SceneBuffer>(
            renderer::rhi::BufferCreationDesc{.usage = renderer::rhi::BufferUsage::ConstantBuffer,
                                              .name = string_to_wstring(scene_name) + L" Scene Buffer"});

        m_game_objects.reserve(Scene::MAX_GAME_OBJECTS);

        core::Log::instance().info(std::format("Created scene {}", scene_name));
    }

    Scene::Scene(const std::string_view scene_name, const uint32_t scene_init_script_index) : Scene(scene_name)
    {
        m_scene_init_script_index = scene_init_script_index;

        scripting::ScriptManager::instance().execute_script(scene_init_script_index);

        sol::table game_objects = scripting::ScriptManager::instance().get_state()["game_objects"];

        for (auto &key_value_pair : game_objects)
        {
            const auto key = key_value_pair.first.as<std::string>();
            const auto value = key_value_pair.second.as<sol::table>();

            const std::string model_path = value["model_path"];

            const math::XMFLOAT3 scale = {value["scale"]["x"], value["scale"]["y"], value["scale"]["z"]};
            const math::XMFLOAT3 rotation = {value["rotation"]["x"], value["rotation"]["y"], value["rotation"]["z"]};
            const math::XMFLOAT3 translation = {value["translation"]["x"], value["translation"]["y"],
                                                value["translation"]["z"]};

            auto new_game_object = create_game_object(key, model_path);

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

            m_game_objects[key] = std::move(new_game_object);
        }

        core::Log::instance().info(std::format("Created scene {}", scene_name));
    }

    GameObject Scene::create_game_object(const std::string_view game_object_name,
                                         const std::string_view gltf_scene_path)
    {
        auto game_object = GameObject{};

        game_object.game_object_index = m_game_objects.size();
        game_object.game_object_name = game_object_name;

        game_object.start_material_index = m_scene_material_buffers_data.size();

        // Load the model data (meshes + materials) and create GPU buffers / textures for them.
        const auto model_data = asset::ModelLoader::load_model(gltf_scene_path);

        auto i = 0;

        auto meshes = std::vector<MeshPart>{};

        for (const auto &mesh_data : model_data.mesh_data)
        {
            // Setup mesh_part.
            auto mesh_part = MeshPart{};

            mesh_part.start_vertex_position = m_scene_positions_data.size();
            mesh_part.start_vertex_normal = m_scene_normals_data.size();
            mesh_part.start_vertex_texture_coord = m_scene_texture_coords_data.size();

            mesh_part.start_index = m_scene_indices.size();
            mesh_part.indices_count = mesh_data.indices.size();

            mesh_part.material_index = mesh_data.material_index;

            meshes.emplace_back(mesh_part);

            // Add data to the scene buffers.
            m_scene_positions_data.insert(m_scene_positions_data.end(), mesh_data.positions.begin(),
                                          mesh_data.positions.end());
            m_scene_normals_data.insert(m_scene_normals_data.end(), mesh_data.normals.begin(), mesh_data.normals.end());
            m_scene_texture_coords_data.insert(m_scene_texture_coords_data.end(), mesh_data.texture_coords.begin(),
                                               mesh_data.texture_coords.end());

            m_scene_indices.insert(m_scene_indices.end(), mesh_data.indices.begin(), mesh_data.indices.end());

            ++i;
        }

        game_object.mesh = meshes.at(0);

        m_mesh_parts.insert(m_mesh_parts.end(), meshes.begin(), meshes.end());

        i = 0;

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
                        .name = string_to_wstring(game_object_name) + L" Albedo Texture Material " + std::to_wstring(i),
                    },
                    reinterpret_cast<const std::byte *>(base_color_texture_data.data()));

                material.albedo_texture_srv_index =
                    renderer::Renderer::instance().get_texture_at_index(albedo_texture_index).srv_index;
            }

            material.base_color = material_data.base_color;
            material.metallic_roughness_factor = material_data.metallic_roughness_factor;

            m_scene_material_buffers_data.push_back(material);

            ++i;
        }

        return game_object;
    }

    void Scene::update(const math::XMMATRIX projection_matrix, const float delta_time, const uint32_t frame_count,
                       const core::Input &input)
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
            .update(reinterpret_cast<const std::byte *>(&m_scene_buffer), sizeof(interop::SceneBuffer));

        for (auto &[name, game_object] : m_game_objects)
        {
            m_scene_buffer.transform_buffer[game_object.game_object_index] =
                game_object.transform_component.update(delta_time, frame_count);
        }
    }

    void Scene::reload()
    {
        if (!m_scene_init_script_index.has_value())
        {
            core::Log::instance().warn(
                std::format("Cannot reload scene with name {} that does not have a scene init script", m_scene_name));
            return;
        }

        m_game_objects.clear();

        scripting::ScriptManager::instance().execute_script(m_scene_init_script_index.value());

        sol::table game_objects = scripting::ScriptManager::instance().get_state()["game_objects"];

        for (auto &key_value_pair : game_objects)
        {
            const auto key = key_value_pair.first.as<std::string>();
            const auto value = key_value_pair.second.as<sol::table>();

            const std::string model_path = value["model_path"];

            const math::XMFLOAT3 scale = {value["scale"]["x"], value["scale"]["y"], value["scale"]["z"]};
            const math::XMFLOAT3 rotation = {value["rotation"]["x"], value["rotation"]["y"], value["rotation"]["z"]};
            const math::XMFLOAT3 translation = {value["translation"]["x"], value["translation"]["y"],
                                                value["translation"]["z"]};

            auto new_game_object = create_game_object(key, model_path);
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

            m_game_objects[key] = (std::move(new_game_object));
        }

        core::Log::instance().info(std::format("Created scene {}", m_scene_name));
    }

    void Scene::add_light(const interop::Light &light)
    {
        m_lights.add_light(light);
    }
} // namespace serenity::scene