#include "serenity-engine/asset/mesh_loader.hpp"

#include "serenity-engine/core/file_system.hpp"
#include "serenity-engine/graphics/device.hpp"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tiny_gltf.h>

namespace serenity::asset::MeshLoader
{
    scene::Mesh load_mesh(const std::string_view mesh_path)
    {
        auto mesh = scene::Mesh{};

        auto warning = std::string{};
        auto error = std::string{};

        auto context = tinygltf::TinyGLTF{};
        auto model = tinygltf::Model{};

        if (!context.LoadASCIIFromFile(&model, &error, &warning,
                                       core::FileSystem::instance().get_relative_path(mesh_path)))
        {
            if (!error.empty())
            {
                core::Log::instance().error(error);
            }

            if (!warning.empty())
            {
                core::Log::instance().warn(warning);
            }
        }

        const auto &node = model.nodes[model.defaultScene];

        const auto &nodeMesh = model.meshes[node.mesh];

        auto indices = std::vector<uint16_t>{};

        // Reference used :
        // https://github.com/mateeeeeee/Adria-DX12/blob/fc98468095bf5688a186ca84d94990ccd2f459b0/Adria/Rendering/EntityLoader.cpp.

        // Get Accesor, buffer view and buffer for each attribute (position, textureCoord, normal).
        auto primitive = nodeMesh.primitives[0];
        const auto &index_accesor = model.accessors[primitive.indices];

        // Position data.
        const auto &position_accesor = model.accessors[primitive.attributes["POSITION"]];
        const auto &position_buffer_view = model.bufferViews[position_accesor.bufferView];
        const auto &position_buffer = model.buffers[position_buffer_view.buffer];

        const auto position_byte_stride = position_accesor.ByteStride(position_buffer_view);
        const auto *positions = &position_buffer.data[position_buffer_view.byteOffset + position_accesor.byteOffset];

        // Get the index buffer data.
        const auto &index_buffer_view = model.bufferViews[index_accesor.bufferView];
        const auto &gltf_index_buffer = model.buffers[index_buffer_view.buffer];
        const auto index_byte_stride = index_accesor.ByteStride(index_buffer_view);
        const auto *indexes = gltf_index_buffer.data.data() + index_buffer_view.byteOffset + index_accesor.byteOffset;

        auto position_data = std::vector<math::XMFLOAT3>{};

        // Fill in the vertices array.
        for (auto i : std::views::iota(0u, position_accesor.count))
        {
            const math::XMFLOAT3 position = {
                (reinterpret_cast<float const *>(positions + (i * position_byte_stride)))[0],
                (reinterpret_cast<float const *>(positions + (i * position_byte_stride)))[1],
                (reinterpret_cast<float const *>(positions + (i * position_byte_stride)))[2],
            };

            position_data.emplace_back(position);
        }

        // Fill indices array.
        for (const auto i : std::views::iota(0u, index_accesor.count))
        {
            if (index_accesor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
            {
                indices.emplace_back(reinterpret_cast<uint16_t const *>(indexes + (i * index_byte_stride))[0]);
            }
            else if (index_accesor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
            {
                indices.emplace_back(reinterpret_cast<uint16_t const *>(indexes + (i * index_byte_stride))[0]);
            }
        }

        mesh.indices_count = static_cast<uint32_t>(indices.size());

        // Create the vertex buffer and index buffer.
        mesh.position_buffer = graphics::Device::instance().create_buffer<math::XMFLOAT3>(
            graphics::BufferCreationDesc{
                .usage = graphics::BufferUsage::StructuredBuffer,
                .name = L"Position Buffer",
            },
            position_data);

        mesh.index_buffer = graphics::Device::instance().create_buffer<uint16_t>(
            graphics::BufferCreationDesc{
                .usage = graphics::BufferUsage::IndexBuffer,
                .name = L"Position Buffer",
            },
            indices);

        core::Log::instance().info(std::format("Loaded mesh from path : {}", mesh_path));

        return mesh;
    }
} // namespace serenity::asset