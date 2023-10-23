#pragma once

#include "camera.hpp"
#include "game_object.hpp"
#include "lights.hpp"

#include "shaders/interop/constant_buffers.hlsli"

namespace serenity::scene
{
    // A collection of game objects, camera, lights and all things related to the scene.
    // cube map, scene buffer(s), etc.
    // If a lua file is used to initialize the scene, then the user can optionally 'reload the scene'..
    class Scene
    {
      public:
        explicit Scene(const std::string_view scene_name);

        // Specify the scene parameters (game objects, etc) in a script and construct the scene from it.
        explicit Scene(const std::string_view scene_name, const uint32_t scene_init_script_index);

        ~Scene() = default;

        void add_game_object(const GameObject &&game_object)
        {
            m_game_objects[game_object.m_game_object_name] = std::move(game_object);
        }

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

      public:
        static constexpr uint32_t MAX_GAME_OBJECTS = 1000u;

      private:
        uint32_t m_scene_buffer_index{};
        interop::SceneBuffer m_scene_buffer{};

        Camera m_camera{};

        Lights m_lights{};

        std::unordered_map<std::string, GameObject> m_game_objects{};

        std::string m_scene_name{};

        std::optional<uint32_t> m_scene_init_script_index{};
    };
} // namespace serenity::scene