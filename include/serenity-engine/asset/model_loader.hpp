#pragma once

#include "serenity-engine/core/singleton_instance.hpp"

namespace serenity::asset
{
    struct MeshData
    {
        std::vector<math::XMFLOAT3> positions{};
        std::vector<math::XMFLOAT3> normals{};
        std::vector<math::XMFLOAT2> texture_coords{};

        std::vector<uint16_t> indices{};

        math::XMMATRIX transform{};
    };

    struct ModelData
    {
        std::vector<MeshData> mesh_data{};
    };

    namespace ModelLoader
    {
        // A utility namespace that helps in loading models / scenes from a gltf file.
        // The output contains a vector of meshes and material data, from which GPU buffers are to be created (not done
        // here). This design is taken so as to reduce dependency between the process of loading GLTF files and actually
        // constructing data from them.
        // Model loader currently uses fastgltf.
        [[nodiscard]] ModelData load_model(const std::string_view model_path,
                                           const math::XMMATRIX transform_matrix = math::XMMatrixIdentity());
    } // namespace ModelLoader
} // namespace serenity::asset