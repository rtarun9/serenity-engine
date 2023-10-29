#include "serenity-engine/scene/game_object.hpp"

namespace serenity::scene
{
    void GameObject::update(const float delta_time, const uint32_t frame_count)
    {
        if (script_index != INVALID_INDEX_U32)
        {
            scripting::ScriptManager::instance().execute_script(script_index);

            auto &transform = transform_component;

            std::tie(transform.scale, transform.rotation, transform.translation) =
                scripting::ScriptManager::instance().call_function()["update_transform"](
                    transform.scale, transform.rotation, transform.translation, delta_time, frame_count);
        }

        transform_component.update(delta_time, frame_count);
    }
} // namespace serenity::scene