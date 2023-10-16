target_sources(serenity-engine PUBLIC
	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/renderpass/atmosphere_renderpass.hpp"
	"atmosphere_renderpass.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/renderpass/cube_map_renderpass.hpp"
	"cube_map_renderpass.cpp"
	
	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/renderpass/shading_renderpass.hpp"
	"shading_renderpass.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/renderpass/post_processing_renderpass.hpp"
	"post_processing_renderpass.cpp"
)