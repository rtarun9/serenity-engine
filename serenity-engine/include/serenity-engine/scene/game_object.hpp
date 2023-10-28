#pragma once

#include "serenity-engine/renderer/renderer.hpp"
#include "shaders/interop/constant_buffers.hlsli"

#include "serenity-engine/scripting/script_manager.hpp"

namespace serenity::scene
{
    // Main reference for GLTF : https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/
    // Main reference for using Fast GLTF :
    // https://github.com/JuanDiegoMontoya/Fwog/blob/ac1b3d867cb56b8e4640bdc03def96eb3ce924e2/example/common/SceneLoader.h

    struct MeshPart
    {
        uint32_t start_vertex_position{};
        uint32_t start_vertex_normal{};
        uint32_t start_vertex_texture_coord{};

        uint32_t start_index{};
        uint32_t indices_count{};

        uint32_t material_index{};
    };

    struct Transform
    {
        math::XMFLOAT3 scale{1.0f, 1.0f, 1.0f};
        math::XMFLOAT3 rotation{0.0f, 0.0f, 0.0f};
        math::XMFLOAT3 translation{0.0f, 0.0f, 0.0f};

        interop::TransformBufferData update(const float delta_time, const uint32_t frame_count)
        {
            const auto model_matrix = math::XMMatrixScaling(scale.x, scale.y, scale.z) *
                                      math::XMMatrixRotationX(math::XMConvertToRadians(rotation.x)) *
                                      math::XMMatrixRotationY(math::XMConvertToRadians(rotation.y)) *
                                      math::XMMatrixRotationZ(math::XMConvertToRadians(rotation.z)) *
                                      math::XMMatrixTranslation(translation.x, translation.y, translation.z);

            const auto transform_buffer = interop::TransformBufferData{
                .model_matrix = model_matrix,
                .transposed_inverse_model_matrix =
                    math::XMMatrixTranspose(math::XMMatrixInverse(nullptr, model_matrix)),
            };

            return transform_buffer;
        }
    };

    struct GameObject
    {
        uint32_t game_object_index{};

        uint32_t script_index{INVALID_INDEX_U32};

        MeshPart mesh{};
        uint32_t start_material_index{};

        Transform transform_component{};

        std::string game_object_name{};
    };
} // namespace serenity::scene