#pragma once

#include "serenity-engine/renderer/renderer.hpp"
#include "shaders/interop/constant_buffers.hlsli"

#include "serenity-engine/scripting/script_manager.hpp"

namespace serenity::scene
{
    // A serenity::GameObject is essentially a abstraction of mesh + materials, transform components, scripts, etc.

    // Main reference for GLTF : https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/
    // Main reference for using Fast GLTF :
    // https://github.com/JuanDiegoMontoya/Fwog/blob/ac1b3d867cb56b8e4640bdc03def96eb3ce924e2/example/common/SceneLoader.h

    struct Mesh
    {
        uint32_t position_buffer_index{};
        uint32_t normal_buffer_index{};
        uint32_t texture_coords_buffer_index{};

        uint32_t index_buffer_index{};

        uint32_t material_index{};

        uint32_t indices{};
    };

    // Note : MaterialBuffer contains some texture indices, which are the actual SRV indices. The material_buffer_index
    // on the other hand is the index of material buffer in the renderer's buffer array.
    struct Material
    {
        interop::MaterialBuffer material_data{};

        uint32_t material_buffer_index{};
    };

    // If script_path is not nullopt, then the scale / rotation / translation variables will be potentially modified in
    // the script.
    struct Transform
    {
        math::XMFLOAT3 scale{1.0f, 1.0f, 1.0f};
        math::XMFLOAT3 rotation{};
        math::XMFLOAT3 translation{};

        uint32_t transform_buffer_index{};

        std::optional<std::string> script_path{};

        void update(const float delta_time)
        {
            if (script_path.has_value())
            {
                scripting::ScriptManager::instance().execute_script(*script_path);

                std::tie(scale, rotation, translation) =
                    scripting::ScriptManager::instance().call_function()["update_transform"](scale, rotation, translation, delta_time);
            }

            const auto model_matrix = math::XMMatrixScaling(scale.x, scale.y, scale.z) *
                                      math::XMMatrixRotationX(math::XMConvertToRadians(rotation.x)) *
                                      math::XMMatrixRotationY(math::XMConvertToRadians(rotation.y)) *
                                      math::XMMatrixRotationZ(math::XMConvertToRadians(rotation.z)) *
                                      math::XMMatrixTranslation(translation.x, translation.y, translation.z);

            const auto transform_buffer_data = interop::TransformBuffer{
                .model_matrix = model_matrix,
                .transposed_inverse_model_matrix =
                    math::XMMatrixTranspose(math::XMMatrixInverse(nullptr, model_matrix)),
            };

            renderer::Renderer::instance()
                .get_buffer_at_index(transform_buffer_index)
                .update(reinterpret_cast<const std::byte *>(&transform_buffer_data), sizeof(interop::TransformBuffer));
        }
    };

    struct GameObject
    {
        // Constructor takes in a gltf scene path which will be used to obtain mesh / material data.
        GameObject(const std::string_view object_name, const std::string_view gltf_scene_path);

        std::vector<Mesh> m_meshes{};
        std::vector<Material> m_materials{};

        Transform m_transform_component{};
        std::string m_game_object_name{};

        Transform &get_transform_component()
        {
            return m_transform_component;
        }
    };
} // namespace serenity::scene