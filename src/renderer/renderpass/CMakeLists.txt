target_sources(serenity-engine PUBLIC
	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/renderpass/atmosphere_renderpass.hpp"
	"atmosphere_renderpass.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/renderer/renderpass/cube_map_renderpass.hpp"
	"cube_map_renderpass.cpp"
)