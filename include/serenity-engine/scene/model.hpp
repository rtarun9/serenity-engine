#pragma once

#include "serenity-engine/renderer/rhi/buffer.hpp"
#include "serenity-engine/renderer/rhi/texture.hpp"

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
        renderer::rhi::Buffer position_buffer{};
        renderer::rhi::Buffer normal_buffer{};
        renderer::rhi::Buffer texture_coords_buffer{};

        renderer::rhi::Buffer index_buffer{};

        uint32_t indices{};
    };

    struct Material
    {
        math::XMFLOAT4 base_color{};
        renderer::rhi::Texture base_color_texture{};
    };

    struct Transform
    {
        math::XMFLOAT3 scale{1.0f, 1.0f, 1.0f};
        math::XMFLOAT3 rotation{};
        math::XMFLOAT3 translation{};

        renderer::rhi::Buffer transform_buffer{};

        void update(const math::XMMATRIX view_projection_matrix)
        {
            const auto model_matrix = math::XMMatrixScaling(scale.x, scale.y, scale.z) *
                                      math::XMMatrixRotationX(math::XMConvertToRadians(rotation.x)) *
                                      math::XMMatrixRotationY(math::XMConvertToRadians(rotation.y)) *
                                      math::XMMatrixRotationZ(math::XMConvertToRadians(rotation.z)) *
                                      math::XMMatrixTranslation(translation.x, translation.y, translation.z);

            const auto mvp_matrix = model_matrix * view_projection_matrix;

            transform_buffer.update(reinterpret_cast<const std::byte *>(&mvp_matrix), sizeof(math::XMMATRIX));
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