#pragma once

#include "rhi/d3d_utils.hpp"

#include <dxcapi.h>

namespace serenity::renderer
{
    struct Shader
    {
        comptr<IDxcBlob> blob{};
    };

    enum class ShaderTypes : uint8_t
    {
        Vertex,
        Pixel,
        Compute,
    };

    inline std::string shader_type_to_string(const ShaderTypes shader_type)
    {
        switch (shader_type)
        {
        case ShaderTypes::Vertex: {
            return "Vertex Shader";
        }
        break;

        case ShaderTypes::Pixel: {

            return "Pixel Shader";
        }
        break;

        case ShaderTypes::Compute: {
            return "Compute Shader";
        }
        break;

        default: {
            return "";
        }
        break;
        }
    }

} // namespace serenity::renderer