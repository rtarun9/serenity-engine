#pragma once

#include "d3d_utils.hpp"

namespace serenity::graphics
{
    enum class TextureUsage : uint8_t
    {
        Depth,
        DepthStencil,
    };

    inline std::string texture_usage_to_string(const TextureUsage &texture_usage)
    {
        switch (texture_usage)
        {
        case TextureUsage::Depth: {
            return "Depth Texture";
        }
        break;

        case TextureUsage::DepthStencil: {
            return "Depth Stencil";
        };
        break;

        default: {
            return "";
        }
        break;
        }
    }

    struct TextureCreationDesc
    {
        TextureUsage usage{};
        DXGI_FORMAT format{};
        Uint2 dimension{};
        std::wstring name{};
    };

    struct Texture
    {
        comptr<ID3D12Resource> resource{};

        // Indices of the resource descriptor into the descriptor heap.
        uint32_t srv_index{};
        uint32_t uav_index{};
        uint32_t rtv_index{};
        uint32_t dsv_index{};

        size_t size_in_bytes{};
    };
} // namespace serenity::graphics