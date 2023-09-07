#pragma once

#include "rhi/d3d_utils.hpp"
#include "shader.hpp"

#include "serenity-engine/core/singleton_instance.hpp"

namespace serenity::renderer
{
    // Shader compiler abstraction (using DXC's C++ Api).
    class ShaderCompiler final : public core::SingletonInstance<ShaderCompiler>
    {
      public:
        explicit ShaderCompiler();
        ~ShaderCompiler();

        [[nodiscard]] Shader compile(const ShaderTypes &shader_type, const std::wstring_view shader_path,
                                     const std::wstring_view entry_point);

      private:
        ShaderCompiler(const ShaderCompiler &other) = delete;
        ShaderCompiler &operator=(const ShaderCompiler &other) = delete;

        ShaderCompiler(ShaderCompiler &&other) = delete;
        ShaderCompiler &operator=(ShaderCompiler &&other) = delete;

      private:
        comptr<IDxcCompiler3> m_compiler{};

        // Used to create include handle and provides interfaces for loading shader to blob, etc.
        comptr<IDxcUtils> m_utils{};
        comptr<IDxcIncludeHandler> m_include_handler{};

        std::wstring m_shader_directory{};
    };
} // namespace serenity::renderer