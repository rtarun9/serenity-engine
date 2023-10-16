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

        // There is an option to NOT throw errors / warnings and just return a empty shader in case of errors.
        // This is because of shader reloading. In the case of re-creating shader for hot reloading, the ignore_error flag can be set
        // so that the function responsible of hot-reloading can use the old pipeline and give warnings to the user.
        // This is to ensure re-loading the app is not required when the new-shader has errors.
        [[nodiscard]] Shader compile(const ShaderCreationDesc &shader_creation_desc, const bool ignore_error = false);

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