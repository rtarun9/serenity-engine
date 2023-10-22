target_sources(serenity-engine PUBLIC
	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/rhi/rhi.hpp"
	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/rhi/d3d_utils.hpp"
	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/rhi/buffer.hpp"
	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/rhi/texture.hpp"
	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/rhi/pipeline.hpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/rhi/command_list.hpp"
	"command_list.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/rhi/command_queue.hpp"
	"command_queue.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/rhi/descriptor_heap.hpp"
	"descriptor_heap.cpp"
	
	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/rhi/swapchain.hpp"
	"swapchain.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/rhi/device.hpp"
	"device.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/rhi/root_signature.hpp"
	"root_signature.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/rhi/command_signature.hpp"
	"command_signature.cpp"
)