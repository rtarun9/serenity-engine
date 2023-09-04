set(SERENITY_ENGINE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include/serenity-engine")

target_sources(serenity-engine PUBLIC
	"${SERENITY_ENGINE_INCLUDE_PATH}/scene/mesh.hpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/scene/camera.hpp"
	"camera.cpp"
)