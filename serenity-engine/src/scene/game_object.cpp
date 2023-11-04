#include "serenity-engine/scene/game_object.hpp"

namespace serenity::scene
{
    void Transform::update(const float delta_time, const uint32_t frame_count)
    {
        const auto model_matrix = math::XMMatrixScaling(scale.x, scale.y, scale.z) *
                                  math::XMMatrixRotationX(math::XMConvertToRadians(rotation.x)) *
                                  math::XMMatrixRotationY(math::XMConvertToRadians(rotation.y)) *
                                  math::XMMatrixRotationZ(math::XMConvertToRadians(rotation.z)) *
                                  math::XMMatrixTranslation(translation.x, translation.y, translation.z);

        transform_buffer_data = interop::TransformBuffer{
            .model_matrix = model_matrix,
            .inverse_model_matrix = math::XMMatrixInverse(nullptr, model_matrix),
        };
    }

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