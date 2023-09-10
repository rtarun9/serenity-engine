#pragma once

#include "serenity-engine/renderer/renderer.hpp"
#include "shaders/interop/constant_buffers.hlsli"

namespace serenity::scene
{
    // A serenity::model is basically a GLTF scene, which consist of nodes having meshes (which may or maynot have
    // material id's), materials, transform component, etc.
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
        MaterialBuffer material_data{};

        uint32_t material_buffer_index{};
    };

    struct Transform
    {
        math::XMFLOAT3 scale{1.0f, 1.0f, 1.0f};
        math::XMFLOAT3 rotation{};
        math::XMFLOAT3 translation{};

        uint32_t transform_buffer_index{};

        void update()
        {
            const auto model_matrix = math::XMMatrixScaling(scale.x, scale.y, scale.z) *
                                      math::XMMatrixRotationX(math::XMConvertToRadians(rotation.x)) *
                                      math::XMMatrixRotationY(math::XMConvertToRadians(rotation.y)) *
                                      math::XMMatrixRotationZ(math::XMConvertToRadians(rotation.z)) *
                                      math::XMMatrixTranslation(translation.x, translation.y, translation.z);

            const auto transform_buffer_data = TransformBuffer{.model_matrix = model_matrix};

            renderer::Renderer::instance()
                .get_buffer_at_index(transform_buffer_index)
                .update(reinterpret_cast<const std::byte *>(&transform_buffer_data), sizeof(TransformBuffer));
        }
    };

    struct Model
    {
        std::vector<Mesh> meshes{};
        std::vector<Material> materials{};

        Transform transform_component{};
        std::string model_name{};
    };
} // namespace serenity::scene