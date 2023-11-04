#pragma once

#include "serenity-engine/core/singleton_instance.hpp"

#include "scene.hpp"

namespace serenity::scene
{
    // A static class that holds a list of scenes and their respective names, and exposes methods to set the current
    // scene.
    // note(rtarun9) : This class is added to the engine quite early since to have the editor be disjoint from the
    // engine, this is required. When the graphics / other features are well developed, this class will be further
    // developed.

    class SceneManager final : public core::SingletonInstance<SceneManager>
    {
      public:
        SceneManager() = default;
        ~SceneManager() = default;

        void add_scene(const Scene &scene);
        void set_current_scene(const std::string_view scene_name);

        Scene &get_current_scene() { return (*(*m_current_scene).second.get()); }

      private:
        std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes{};
        std::unordered_map<std::string, std::unique_ptr<Scene>>::iterator m_current_scene{};
    };
} // namespace serenity::scene