#include "serenity-engine/asset/model_loader.hpp"

#include "serenity-engine/core/file_system.hpp"

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

    // Helper function to get transform matrix from node.
    // Reference :
    // https://github.com/spnda/fastgltf/blob/ee5dd8948f2b8191cda01556c7daab5d2799840f/examples/gl_viewer/gl_viewer.cpp#L261
    math::XMMATRIX get_transform_matrix_from_node(const fastgltf::Node &node)
    {
        auto transform = math::XMMatrixIdentity();

        if (auto *trs = std::get_if<fastgltf::Node::TRS>(&node.transform))
        {
            const auto rotation_quaternion = math::XMMatrixRotationQuaternion(
                math::XMVectorSet(trs->rotation[0], trs->rotation[1], trs->rotation[2], trs->rotation[3]));

            transform = math::XMMatrixScaling(trs->scale[0], trs->scale[1], trs->scale[2]) * rotation_quaternion *
                        math::XMMatrixTranslation(trs->translation[0], trs->translation[1], trs->translation[2]);
        }

        else if (auto *mat = std::get_if<fastgltf::Node::TransformMatrix>(&node.transform))
        {
            const auto &m = *mat;
            transform = math::XMMatrixSet(m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11],
                                          m[12], m[13], m[14], m[15]);
        }

        return transform;
    }

    // Function to get mesh data.
    std::vector<MeshData> get_mesh_data_from_node(const fastgltf::Asset &asset, const fastgltf::Node &node)
    {
        auto result_mesh_data = std::vector<MeshData>{};

        // Load all child nodes.
        for (const auto &child_node : node.children)
        {
            const auto child_mesh_data = get_mesh_data_from_node(asset, asset.nodes.at(child_node));
            result_mesh_data.insert(result_mesh_data.end(), child_mesh_data.begin(), child_mesh_data.end());
        }

        // Get GLTF mesh data.
        if (node.meshIndex.has_value())
        {
            const auto &mesh = asset.meshes.at(node.meshIndex.value());

            for (auto &primitive : mesh.primitives)
            {
                auto mesh_data = MeshData{};
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

                if (primitive.materialIndex.has_value())
                {
                    mesh_data.material_index = primitive.materialIndex.value();
                }
                else
                {
                    mesh_data.material_index = 0;
                }
                result_mesh_data.emplace_back(std::move(mesh_data));
            }
        }

        return result_mesh_data;
    }

    // Main reference : https://github.com/spnda/fastgltf/blob/main/examples/gl_viewer/gl_viewer.cpp.
    std::vector<MaterialData> get_material_data_from_asset(const fastgltf::Asset &asset, const std::string path)
    {
        auto result_material_data = std::vector<MaterialData>{};

        for (const auto &material : asset.materials)
        {
            auto material_data = MaterialData{};

            material_data.base_color = math::XMFLOAT4{
                material.pbrData.baseColorFactor[0],
                material.pbrData.baseColorFactor[1],
                material.pbrData.baseColorFactor[2],
                material.pbrData.baseColorFactor[3],
            };

            if (material.pbrData.baseColorTexture.has_value())
            {
                const auto &base_color_texture_info = material.pbrData.baseColorTexture.value();
                auto &base_color_texture = asset.textures.at(base_color_texture_info.textureIndex);

                const auto base_image_index = base_color_texture.imageIndex.has_value()
                                                  ? base_color_texture.imageIndex.value()
                                                  : base_color_texture.fallbackImageIndex.value();

                const auto &base_color_image = asset.images.at(base_image_index);

                if (const auto &texture_path = std::get_if<fastgltf::sources::URI>(&base_color_image.data))
                {
                    material_data.base_color_texture =
                        TextureLoader::load_texture(path + "/"s + texture_path->uri.path().data(), 4u);

                    auto x = material_data.base_color_texture.dimension;
                    auto y = 3;
                }
                else if (const auto &texture_data = std::get_if<fastgltf::sources::Vector>(&base_color_image.data))
                {
                    material_data.base_color_texture = TextureLoader::load_texture(
                        reinterpret_cast<const std::byte *>(texture_data->bytes.data()), texture_data->bytes.size());

                    auto x = material_data.base_color_texture.dimension;
                    auto y = 3;
                }
            }

            result_material_data.emplace_back(material_data);
        }

        return result_material_data;
    }

    ModelData load_model(const std::string_view model_path)
    {
        auto model = ModelData{};

        auto data = fastgltf::GltfDataBuffer();
        const auto path = std::filesystem::path(core::FileSystem::instance().get_absolute_path(model_path));

        if (!data.loadFromFile(path))
        {
            core::Log::instance().critical(std::format("Failed to load GLTF data from model with path : ", model_path));
        }

        // These options tell fastgltf that we want it to load all external buffers, images, and GLB buffers into CPU
        // memory.
        constexpr auto options = fastgltf::Options::LoadExternalBuffers | fastgltf::Options::LoadGLBBuffers |
                                 fastgltf::Options::LoadExternalImages;

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

        // Load meshes for all nodes.
        for (const auto &node : asset.nodes)
        {
            const auto data = get_mesh_data_from_node(asset, node);
            model.mesh_data.insert(model.mesh_data.end(), data.begin(), data.end());
        }

        // Load material data.
        model.material_data = get_material_data_from_asset(asset, path.parent_path().string());

        core::Log::instance().info(std::format("Loaded model from path :  {}", model_path));

        return model;
    }
} // namespace serenity::asset::ModelLoader