#include "serenity-engine/asset/model_loader.hpp"

#include "serenity-engine/core/file_system.hpp"
#include "serenity-engine/graphics/device.hpp"

#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

// Since fastgltf does not provide template specializations for directxmath, we need to manually provide template
// specializations.
namespace fastgltf
{
    template <>
    struct fastgltf::ElementTraits<math::XMFLOAT3>
        : fastgltf::ElementTraitsBase<math::XMFLOAT3, fastgltf::AccessorType::Vec3, float>
    {
    };

    template <>
    struct fastgltf::ElementTraits<math::XMFLOAT2>
        : fastgltf::ElementTraitsBase<math::XMFLOAT2, fastgltf::AccessorType::Vec2, float>
    {
    };
} // namespace fastgltf

using namespace math;

namespace serenity::asset::ModelLoader
{
    // Helper function to get data given the asset and accessor.
    template <typename T>
    std::vector<T> get_data_from_accessor(const fastgltf::Asset &asset, const fastgltf::Accessor &accessor)
    {
        auto attribute_data = std::vector<T>(accessor.count);

        fastgltf::iterateAccessorWithIndex<T>(asset, accessor,
                                              [&](T attribute, size_t index) { attribute_data[index] = attribute; });

        return attribute_data;
    }

    ModelData load_model(const std::string_view model_path, const math::XMMATRIX transform_matrix)
    {
        auto model = ModelData{};

        auto data = fastgltf::GltfDataBuffer();
        if (!data.loadFromFile(core::FileSystem::instance().get_relative_path(model_path)))
        {
            core::Log::instance().critical(std::format("Failed to load GLTF data from model with path : ", model_path));
        }

        auto path = std::filesystem::path(core::FileSystem::instance().get_relative_path(model_path));

        // These options tell fastgltf that we want it to load all external buffers, images, and GLB buffers into cpu
        // memory.
        constexpr auto options = fastgltf::Options::LoadExternalBuffers | fastgltf::Options::LoadExternalImages |
                                 fastgltf::Options::LoadGLBBuffers;

        // Create a parser and parse the GLTF.
        auto parser = fastgltf::Parser();
        auto gltf = fastgltf::Expected<fastgltf::Asset>(fastgltf::Asset{});

        if (path.extension() == ".gltf")
        {
            gltf = parser.loadGLTF(&data, path.parent_path(), options);
        }
        else if (path.extension() == ".glb")
        {
            gltf = parser.loadBinaryGLTF(&data, path.parent_path(), options);
        }
        else
        {
            core::Log::instance().critical(
                std::format("GLTF file extension {} is unsupported. The supported types are : GLTF and GLB",
                            path.extension().string()));
        }

        // Check for errors.
        if (const auto error = gltf.error(); error != fastgltf::Error::None)
        {
            core::Log::instance().critical(std::format("Error while loading model {}. GLTF error code : {}", model_path,
                                                       static_cast<uint32_t>(error)));
        }

        const auto &asset = gltf.get();
        if (asset.scenes.size() > 1)
        {
            core::Log::instance().warn(
                "For now, only gltf's with single scene are loaded. This will be implemented in future");
        }

        // Get all nodes from scene so they can be traversed (DFS).
        auto node_indices = std::stack<size_t>{};
        for (auto node_index : asset.scenes[asset.defaultScene.value()].nodeIndices)
        {
            node_indices.emplace(node_index);
        }

        while (!node_indices.empty())
        {
            auto node_index = node_indices.top();
            node_indices.pop();

            const auto &node = asset.nodes.at(node_index);

            // Load child nodes.
            for (auto &child_nodes : node.children)
            {
                node_indices.emplace(child_nodes);
            }

            // Check if the current node has a mesh.
            if (node.meshIndex.has_value())
            {
                auto mesh_data = MeshData{};
                const auto &mesh = asset.meshes.at(node.meshIndex.value());

                for (const auto &primitive : mesh.primitives)
                {
                    // Load attributes.

                    // Load positions.
                    auto &position_accessor = asset.accessors[primitive.findAttribute("POSITION")->second];
                    mesh_data.positions = get_data_from_accessor<math::XMFLOAT3>(asset, position_accessor);

                    // Load normals.
                    auto &normal_accessor = asset.accessors[primitive.findAttribute("NORMAL")->second];
                    mesh_data.normals = get_data_from_accessor<math::XMFLOAT3>(asset, normal_accessor);

                    // Load texture coords.
                    auto &texture_coord_accessor = asset.accessors[primitive.findAttribute("TEXCOORD_0")->second];
                    mesh_data.texture_coords = get_data_from_accessor<math::XMFLOAT2>(asset, texture_coord_accessor);

                    // Load index buffer.
                    auto &index_accessor = asset.accessors[primitive.indicesAccessor.value()];
                    mesh_data.indices = get_data_from_accessor<uint16_t>(asset, index_accessor);
                }

                model.mesh_data.emplace_back(std::move(mesh_data));
            };
        }

        core::Log::instance().info(std::format("Loaded model from path :  {}", model_path));

        return model;
    }
} // namespace serenity::asset::ModelLoader