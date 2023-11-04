#pragma once

#include "d3d_utils.hpp"

namespace serenity::renderer::rhi
{
    // Structured buffer is GPU only, while the dynamic structured buffer has CPU write access.
    enum class BufferUsage : uint8_t
    {
        ConstantBuffer,
        StructuredBuffer,
        DynamicStructuredBuffer,
        UAVBuffer,
        IndexBuffer,
    };

    inline std::string buffer_usage_to_string(const BufferUsage &buffer_usage)
    {
        switch (buffer_usage)
        {
        case BufferUsage::ConstantBuffer: {
            return "Constant Buffer";
        }
        break;

        case BufferUsage::StructuredBuffer: {
            return "Structured Buffer";
        }
        break;

        case BufferUsage::DynamicStructuredBuffer: {
            return "Dynamic Structured Buffer";
        }
        break;

        case BufferUsage::UAVBuffer: {
            return "UAV Buffer";
        }
        break;

        case BufferUsage::IndexBuffer: {
            return "Index Buffer";
        }
        break;

        default: {
            return "";
        }
        break;
        }
    }

    struct BufferCreationDesc
    {
        BufferUsage usage{};
        std::wstring name{};
    };

    struct Buffer
    {
        comptr<ID3D12Resource> resource{};

        // Indices of the resource descriptor into the descriptor heap.
        uint32_t cbv_index{};
        uint32_t srv_index{};
        uint32_t uav_index{};

        size_t size_in_bytes{};

        // To be used only by constant buffers.
        std::optional<uint8_t *> mapped_pointer{};

        void update(const std::byte *data, const size_t size)
        {
            std::memcpy(mapped_pointer.value(), reinterpret_cast<const void *>(data), size);
        }
    };
} // namespace serenity::renderer::rhi