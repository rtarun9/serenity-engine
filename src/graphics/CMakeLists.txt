set(SERENITY_ENGINE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include/serenity-engine")

target_sources(serenity-engine PUBLIC
	"${SERENITY_ENGINE_INCLUDE_PATH}/graphics/d3d_utils.hpp"
	"${SERENITY_ENGINE_INCLUDE_PATH}/graphics/buffer.hpp"
	"${SERENITY_ENGINE_INCLUDE_PATH}/graphics/texture.hpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/graphics/command_list.hpp"
	"command_list.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/graphics/command_queue.hpp"
	"command_queue.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/graphics/descriptor_heap.hpp"
	"descriptor_heap.cpp"
	
	"${SERENITY_ENGINE_INCLUDE_PATH}/graphics/swapchain.hpp"
	"swapchain.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/graphics/device.hpp"
	"device.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/graphics/shader_compiler.hpp"
	"shader_compiler.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/graphics/root_signature.hpp"
	"root_signature.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/graphics/pipeline.hpp"
	"pipeline.cpp"
)