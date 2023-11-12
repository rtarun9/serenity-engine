#pragma once

#include "camera.hpp"
#include "game_object.hpp"
#include "lights.hpp"

#include "shaders/interop/constant_buffers.hlsli"

namespace serenity::scene
{
    // A collection of game objects, camera, lights and all things related to the scene.
    // For simplicity, the only way to add game objects is via a lua scene script.
    // Since gpu driven rendering is done, scene will contain a large position / normal / material etc buffers
    // which are internally arrays.
    // The GameObject's can index into these 'global' buffers and access their respective elements.

    // Struct of all the scene - global resources (scene mesh buffer, material buffer, position buffer, etc).
    struct SceneResources
    {
        uint32_t scene_buffer_index{};
        interop::SceneBuffer scene_buffer{};

        uint32_t position_buffer_index{};
        std::vector<math::XMFLOAT3> positions{};

        uint32_t normal_buffer_index{};
        std::vector<math::XMFLOAT3> normals{};

        uint32_t texture_coord_buffer_index{};
        std::vector<math::XMFLOAT2> texture_coords{};

        uint32_t index_buffer_index{};
        std::vector<uint16_t> indices{};

        uint32_t materal_buffer_index{};
        std::vector<interop::MaterialBuffer> material_buffers{};

        uint32_t meshes_buffer_index{};
        std::vector<interop::MeshBuffer> mesh_buffers{};

        uint32_t game_object_buffer_index{};
        std::vector<interop::GameObjectBuffer> game_object_buffers{};
    };

    class Scene
    {
      public:
        // Specify the scene parameters (game objects, etc) in a script and construct the scene from it.
        explicit Scene(const std::string_view scene_name, const std::string_view scene_init_script_path);

        SceneResources &get_scene_resources() { return m_scene_resources; }

        std::optional<uint32_t> get_scene_init_script_index() const { return m_scene_init_script_index; }

        const std::string &get_scene_name() const { return m_scene_name; }

        Camera &get_camera() { return m_camera; }

        std::unordered_map<std::string, GameObject> &get_game_objects() { return m_game_objects; }
        GameObject &get_game_object(const std::string_view name) { return m_game_objects[name.data()]; }

        Lights &get_lights() { return m_lights; }

        void add_light(const interop::Light &light) { m_lights.add_light(light); }

        void reload();

        // Update the transform component of all game objects in the scene, as well as the scene buffer and camera.
        void update(const math::XMMATRIX projection_matrix, const float delta_time, const uint32_t frame_count,
                    const core::Input &input);

      private:
        // note(rtarun9) : Assumes that scene init script index has a value.
        void load_scene_from_script();

        void create_scene_buffers();

        GameObject create_game_object(const std::string_view game_object_name, const std::string_view gltf_scene_path);

      public:
        static constexpr uint32_t MAX_GAME_OBJECTS = 100u;

      private:
        SceneResources m_scene_resources{};

        Camera m_camera{};

        Lights m_lights{};

        std::unordered_map<std::string, GameObject> m_game_objects{};

        uint32_t m_scene_init_script_index{};

        std::string m_scene_name{};
    };
} // namespace serenity::scene