#pragma once

#include "serenity-engine/renderer/renderer.hpp"
#include "shaders/interop/structured_buffers.hlsli"

#include "serenity-engine/scripting/script_manager.hpp"

namespace serenity::scene
{
    // Main reference for GLTF : https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/
    // Main reference for using Fast GLTF :
    // https://github.com/JuanDiegoMontoya/Fwog/blob/ac1b3d867cb56b8e4640bdc03def96eb3ce924e2/example/common/SceneLoader.h

    // A game object is composed of a collecion of meshes, materials, script index, and transform buffer.
    // Since indirect rendering is used in the engine, the game object's material / meshes will be stored in the scene's
    // vector of material / meshes.
    struct Transform
    {
        math::XMFLOAT3 scale{1.0f, 1.0f, 1.0f};
        math::XMFLOAT3 rotation{0.0f, 0.0f, 0.0f};
        math::XMFLOAT3 translation{0.0f, 0.0f, 0.0f};

        interop::TransformBuffer transform_buffer_data{};

        void update(const float delta_time, const uint32_t frame_count)
        {
            const auto model_matrix = math::XMMatrixScaling(scale.x, scale.y, scale.z) *
                                      math::XMMatrixRotationX(math::XMConvertToRadians(rotation.x)) *
                                      math::XMMatrixRotationY(math::XMConvertToRadians(rotation.y)) *
                                      math::XMMatrixRotationZ(math::XMConvertToRadians(rotation.z)) *
                                      math::XMMatrixTranslation(translation.x, translation.y, translation.z);

            transform_buffer_data = interop::TransformBuffer{
                .model_matrix = model_matrix,
                .inverse_model_matrix =
                    math::XMMatrixInverse(nullptr, model_matrix),
            };
        }
    };

    struct GameObject
    {
        uint32_t game_object_index{};
        std::string game_object_name{};

        uint32_t script_index{INVALID_INDEX_U32};
        Transform transform_component{};

        uint32_t mesh_buffer_offset{};
        uint32_t material_buffer_offset{};

        uint32_t mesh_count{};
        uint32_t material_count{};

        void update(const float delta_time, const uint32_t frame_count);
    };
} // namespace serenity::scene