#pragma once

#include "texture_loader.hpp"

namespace serenity::asset
{
    // Mesh / Material data is the data extracted after processing the gltf / image files.
    // The actual GPU buffer will be created by relevant abstractions.
    struct MeshData
    {
        std::vector<math::XMFLOAT3> positions{};
        std::vector<math::XMFLOAT3> normals{};
        std::vector<math::XMFLOAT2> texture_coords{};

        std::vector<uint16_t> indices{};

        math::XMMATRIX mesh_local_transform_matrix{};
        math::XMMATRIX inverse_mesh_local_transform_matrix{};

        uint32_t material_index{};
    };

    struct MaterialData
    {
        math::XMFLOAT4 base_color{};
        math::XMFLOAT2 metallic_roughness_factor{};
        TextureData base_color_texture{};
    };

    struct ModelData
    {
        std::vector<MeshData> mesh_data{};
        std::vector<MaterialData> material_data{};
    };

    namespace ModelLoader
    {
        // A utility namespace that helps in loading models / scenes from a gltf file.
        // The output contains a vector of meshes and material data, from which GPU buffers are to be created (not done
        // here). This design is taken so as to reduce dependency between the process of loading GLTF files and actually
        // constructing data from them.
        // Model loader currently uses fastgltf.
        [[nodiscard]] ModelData load_model(const std::string_view model_path);
    } // namespace ModelLoader
} // namespace serenity::asset