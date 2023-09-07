target_sources(serenity-engine PUBLIC
	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/shader.hpp"
	
	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/renderer.hpp"
	"renderer.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/shader_compiler.hpp"
	"shader_compiler.cpp"
)

add_subdirectory(rhi)