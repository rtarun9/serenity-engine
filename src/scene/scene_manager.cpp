#include "serenity-engine/scene/scene_manager.hpp"

namespace serenity::scene
{
    void SceneManager::add_scene(Scene &&scene)
    {
        m_scenes[scene.get_scene_name()] = std::make_unique<Scene>(scene);

        // If this is the first scene to be added, make it the current scene.
        if (m_scenes.size() == 1)
        {
            m_current_scene = m_scenes.begin();
        }
    }

    void SceneManager::set_current_scene(const std::string_view scene_name)
    {
        auto scene = m_scenes.find(std::string(scene_name));

        if (scene == m_scenes.end())
        {
            if (m_scenes.size() != 0)
            {
                core::Log::instance().warn(std::format("{} is not a valid scene name, setting current scene to {}",
                                                       scene_name, (*m_scenes.begin()).first));
                m_current_scene = m_scenes.begin();
            }
            else
            {
                core::Log::instance().error(std::format("{} is not a valid scene name, and since no scenes are added "
                                                        "to scene manager, engine is terminating",
                                                        scene_name));
            }
        }
        else
        {
            // We found a scene with name 'scene_name' in the unordered_map.
            m_current_scene = scene;
        }
    }
} // namespace serenity::scene