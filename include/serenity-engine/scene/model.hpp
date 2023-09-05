#pragma once

#include "serenity-engine/graphics/buffer.hpp"
#include "serenity-engine/graphics/texture.hpp"

namespace serenity::scene
{
    // A serenity::model is basically a GLTF scene, which consist of nodes having meshes (which may or maynot have
    // material id's), materials, etc.
    // Main reference for GLTF : https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/
    // Main reference for using Fast GLTF :
    // https://github.com/JuanDiegoMontoya/Fwog/blob/ac1b3d867cb56b8e4640bdc03def96eb3ce924e2/example/common/SceneLoader.h

    struct Mesh
    {
        graphics::Buffer position_buffer{};
        graphics::Buffer normal_buffer{};
        graphics::Buffer texture_coords_buffer{};

        graphics::Buffer index_buffer{};

        uint32_t indices{};
    };

    struct Material
    {
        math::XMFLOAT4 base_color{};
        graphics::Texture base_color_texture{};
    };

    struct Model
    {
        std::vector<Mesh> meshes{};
        std::vector<Material> materials{};
    };
} // namespace serenity::scene