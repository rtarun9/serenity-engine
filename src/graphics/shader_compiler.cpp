#include "serenity-engine/graphics/shader_compiler.hpp"

#include "serenity-engine/core/file_system.hpp"

namespace serenity::graphics
{
    ShaderCompiler::ShaderCompiler()
    {
        // Init core DXC objects.
        throw_if_failed(::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils)));
        throw_if_failed(::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler)));
        throw_if_failed(m_utils->CreateDefaultIncludeHandler(&m_include_handler));

        // Get the root directory.
        m_root_directory = string_to_wstring(core::FileSystem::instance().get_root_directory());

        core::Log::instance().info("Created shader compiler");
    }

    ShaderCompiler::~ShaderCompiler()
    {
        core::Log::instance().info("Destroyed shader compiler");
    }

    Shader ShaderCompiler::compile(const ShaderTypes &shader_type, const std::wstring_view shader_path,
                                   const std::wstring_view entry_point)
    {
        auto shader = Shader{};

        // Setup compilation arguments.
        const auto target_profile = [=]() -> std::wstring {
            switch (shader_type)
            {
            case ShaderTypes::Vertex: {
                return L"vs_6_6";
            }
            break;

            case ShaderTypes::Pixel: {
                return L"ps_6_6";
            }
            break;

            default: {
                return L"";
            }
            break;
            }
        }();

        auto compilation_args = std::vector<LPCWSTR>{
            L"-E",
            entry_point.data(),
            L"-T",
            target_profile.c_str(),
            DXC_ARG_PACK_MATRIX_ROW_MAJOR,
            DXC_ARG_WARNINGS_ARE_ERRORS,
            DXC_ARG_ALL_RESOURCES_BOUND,
            L"-Qstrip_debug",
            L"-Qstrip_reflect",
            L"-I",
            m_root_directory.c_str(),
        };

        // Indicate that the shader should be in a debuggable state if in debug mode.
        // Else, set optimization level to 03.
        if constexpr (SERENITY_DEBUG)
        {
            compilation_args.push_back(DXC_ARG_DEBUG);
        }
        else
        {
            compilation_args.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
        }

        // Load the shader source file to a blob.
        auto source_blob = comptr<IDxcBlobEncoding>{};

        const auto full_shader_path =
            string_to_wstring(core::FileSystem::instance().get_relative_path(wstring_to_string(shader_path)));

        throw_if_failed(m_utils->LoadFile(full_shader_path.data(), nullptr, &source_blob));

        const auto source_buffer = DxcBuffer{
            .Ptr = source_blob->GetBufferPointer(),
            .Size = source_blob->GetBufferSize(),
            .Encoding = 0u,
        };

        // Compile the shader.
        auto compiled_shader_buffer = comptr<IDxcResult>{};
        const auto hr =
            m_compiler->Compile(&source_buffer, compilation_args.data(), static_cast<uint32_t>(compilation_args.size()),
                                m_include_handler.Get(), IID_PPV_ARGS(&compiled_shader_buffer));
        if (FAILED(hr))
        {
            core::Log::instance().critical(
                std::format("Failed to compile shader with path : {}", wstring_to_string(shader_path)));
        }

        // Get compilation errors (if any).
        auto errors = comptr<IDxcBlobUtf8>{};
        throw_if_failed(compiled_shader_buffer->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
        if (errors && errors->GetStringLength() > 0)
        {
            const auto error_message = errors->GetStringPointer();
            core::Log::instance().critical(
                std::format("Shader path : {}, Error : {}", wstring_to_string(shader_path), error_message));
        }

        auto compiled_shader_blob = comptr<IDxcBlob>{};
        throw_if_failed(compiled_shader_buffer->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiled_shader_blob), nullptr));

        shader.blob = compiled_shader_blob;

        const auto shader_type_str = [&](const auto shader_type) -> std::string {
            switch (shader_type)
            {
            case ShaderTypes::Vertex: {
                return "Vertex";
            }
            break;

            case ShaderTypes::Pixel: {
                return "Pixel";
            }
            break;

            default: {
                return "";
            }
            break;
            };
        };

        core::Log::instance().info(std::format("Compiled {} shader with path : {}", shader_type_str(shader_type),
                                               wstring_to_string(shader_path)));
        return shader;
    }
} // namespace serenity::graphics