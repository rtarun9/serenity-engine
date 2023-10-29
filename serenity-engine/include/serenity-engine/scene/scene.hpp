#pragma once

#include "camera.hpp"
#include "game_object.hpp"
#include "lights.hpp"

#include "shaders/interop/constant_buffers.hlsli"

namespace serenity::scene
{
    // A collection of game objects, camera, lights and all things related to the scene.
    // cube map, scene buffer(s), etc.
    // If a lua file is used to initialize the scene, then the user can optionally 'reload the scene'.
    // Since gpu driven rendering is done, scene will contain a large position / normal / material etc buffers
    // which are internally arrays.
    // The GameObject's can index into these 'global' buffers and access their respective elements.
    class Scene
    {
      public:
        explicit Scene(const std::string_view scene_name);

        // Specify the scene parameters (game objects, etc) in a script and construct the scene from it.
        explicit Scene(const std::string_view scene_name, const uint32_t scene_init_script_index);

        ~Scene() = default;

        GameObject create_game_object(const std::string_view game_object_name, const std::string_view gltf_scene_path);

        uint32_t get_scene_buffer_index() const
        {
            return m_scene_buffer_index;
        }

        std::optional<uint32_t> get_scene_init_script_index() const
        {
            return m_scene_init_script_index;
        }

        const std::string &get_scene_name()
        {
            return m_scene_name;
        }

        Camera &get_camera()
        {
            return m_camera;
        }

        interop::SceneBuffer &get_scene_buffer()
        {
            return m_scene_buffer;
        }

        std::unordered_map<std::string, GameObject> &get_game_objects()
        {
            return m_game_objects;
        }

        Lights &get_lights()
        {
            return m_lights;
        }

        GameObject &get_game_object(const std::string_view name)
        {
            return m_game_objects[name.data()];
        }

        void reload();

        void add_light(const interop::Light &light);

        // Update the transform component of all game objects in the scene, as well as the scene buffer and camera.
        void update(const math::XMMATRIX projection_matrix, const float delta_time, const uint32_t frame_count,
                    const core::Input &input);

        // Create all buffers that the scene abstraction contains.
        void create_scene_buffers();

      public:
        static constexpr uint32_t MAX_GAME_OBJECTS = 100u;

      public:
        uint32_t m_scene_buffer_index{};
        interop::SceneBuffer m_scene_buffer{};

        std::vector<math::XMFLOAT3> m_scene_positions_data{};
        uint32_t m_scene_position_buffer_index{};

        std::vector<math::XMFLOAT3> m_scene_normals_data{};
        uint32_t m_scene_normal_buffer_index{};

        std::vector<math::XMFLOAT2> m_scene_texture_coords_data{};
        uint32_t m_scene_texture_coords_buffer_index{};

        std::vector<uint16_t> m_scene_indices{};
        uint32_t m_scene_index_buffer_index{};

        std::vector<interop::MaterialBuffer> m_scene_material_buffer_data{};
        uint32_t m_scene_materal_buffer_index{};

        std::vector<interop::GameObjectBuffer> m_scene_game_object_buffer_data{};
        uint32_t m_scene_game_object_buffer_index{};

        std::vector<interop::MeshBuffer> m_scene_meshes_data{};
        uint32_t m_scene_meshes_buffer_index{};

        Camera m_camera{};

        Lights m_lights{};

        std::unordered_map<std::string, GameObject> m_game_objects{};

        std::string m_scene_name{};

        std::optional<uint32_t> m_scene_init_script_index{};
    };
} // namespace serenity::scene