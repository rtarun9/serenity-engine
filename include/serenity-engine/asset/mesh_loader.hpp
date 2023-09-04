#pragma once

#include "serenity-engine/core/singleton_instance.hpp"
#include "serenity-engine/scene/mesh.hpp"

namespace serenity::asset
{
    // A (work in progress) static class that helps in loading meshes from a gltf file.
    class MeshLoader final : public core::SingletonInstance<MeshLoader>
    {
      public:
        [[nodiscard]] scene::Mesh load_mesh(const std::string_view mesh_path);
    };
} // namespace serenity::asset