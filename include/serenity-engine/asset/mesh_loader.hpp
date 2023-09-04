#pragma once

#include "serenity-engine/core/singleton_instance.hpp"
#include "serenity-engine/scene/mesh.hpp"

namespace serenity::asset
{
    // A (work in progress) utility namespace that helps in loading meshes from a gltf file.
    namespace MeshLoader
    {
        [[nodiscard]] scene::Mesh load_mesh(const std::string_view mesh_path);
    }
} // namespace serenity::asset