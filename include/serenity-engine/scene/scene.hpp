#pragma once

#include "camera.hpp"
#include "model.hpp"
#include "lights.hpp"

#include "shaders/interop/constant_buffers.hlsli"

namespace serenity::scene
{
    // Explanation of the 'structure' of a scene.
    // The engine supports only GLTF models, which in gltf terms are scenes : A collection of nodes with cameras, meshes
    // with primitives and transformations, and those meshes can have material id's. In the engine, this GLTF scene is a
    // model of meshes and materials, while a serenity::Scene is a collection of models, a scene camera, environment /
    // cube map, scene buffer(s), etc.
    class Scene
    {
      public:
        explicit Scene(const std::string_view scene_name);
        ~Scene() = default;

        uint32_t get_scene_buffer_index() const
        {
            return m_scene_buffer_index;
        }

        const std::string &get_scene_name()
        {
            return m_scene_name;
        }

        Camera &get_camera()
        {
            return m_camera;
        }

        SceneBuffer &get_scene_buffer()
        {
            return m_scene_buffer;
        }

        std::vector<Model> &get_models()
        {
            return m_models;
        }

        LightBuffer& get_light_buffer()
        {
            return m_lights.get_light_buffer();
        }

        uint32_t get_light_buffer_index() const
        {
            return m_lights.get_light_buffer_index();
        }

        void add_model(const std::string_view model_path, const std::string_view model_name,
                       const math::XMFLOAT3 scale = math::XMFLOAT3{1.0f, 1.0f, 1.0f});

        // Update the transform component of all models in the scene, as well as the scene buffer and camera.
        void update(const math::XMMATRIX projection_matrix, const float delta_time, const core::Input &input);

      private:
        uint32_t m_scene_buffer_index{};
        SceneBuffer m_scene_buffer{};

        Camera m_camera{};

        Lights m_lights{};

        std::vector<Model> m_models{};

        std::string m_scene_name{};
    };
} // namespace serenity::scene