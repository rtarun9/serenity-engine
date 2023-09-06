#pragma once

#include "camera.hpp"
#include "model.hpp"

namespace serenity::scene
{
    // Explanation of the 'structure' of a scene.
    // The engine supports only GLTF models, which in gltf terms are scenes : A collection of nodes with cameras, meshes
    // with primitives and transformations, and those meshes can have material id's. In the engine, this GLTF scene is a
    // model of meshes and materials, while a serenity::Scene is a collection of models, a scene camera, environment /
    // cube map, etc.
    class Scene
    {
      public:
        explicit Scene(const std::string_view scene_name);
        ~Scene() = default;

        void add_model(const std::string_view model_path, const std::string_view model_name);

        // Update the transform component of all models in the scene.
        void update(const math::XMMATRIX projection_matrix);

        const std::string &get_scene_name()
        {
            return m_scene_name;
        }

        Camera &get_camera()
        {
            return m_camera;
        }

        std::vector<Model> &get_models()
        {
            return m_models;
        }

      private:
        std::string m_scene_name{};

        Camera m_camera{};

        std::vector<Model> m_models{};
    };
} // namespace serenity::scene