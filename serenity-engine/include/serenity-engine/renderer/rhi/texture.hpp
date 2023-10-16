#pragma once

#include "d3d_utils.hpp"

namespace serenity::renderer::rhi
{
    enum class TextureUsage : uint8_t
    {
        DepthStencilTexture,
        ShaderResourceTexture,
        UAVTexture,
        RenderTexture,
    };

    inline std::string texture_usage_to_string(const TextureUsage &texture_usage)
    {
        switch (texture_usage)
        {
        case TextureUsage::DepthStencilTexture: {
            return "Depth Stencil Texture";
        };
        break;

        case TextureUsage::ShaderResourceTexture: {
            return "Shader Resource Texture";
        }
        break;

        case TextureUsage::UAVTexture: {

            return "UAV Texture";
        }
        break;

        case TextureUsage::RenderTexture: {
            return "Render Texture";
        }
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
        uint32_t mip_levels{1u};
        uint32_t num_channels{4u};
        uint32_t bytes_per_pixel{4u};
        uint32_t array_size{1u};   
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
    };
} // namespace serenity::renderer::rhi