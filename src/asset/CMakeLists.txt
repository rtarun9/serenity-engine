set(SERENITY_ENGINE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include/serenity-engine/")

target_sources(serenity-engine PUBLIC
	"${SERENITY_ENGINE_INCLUDE_PATH}/asset/mesh_loader.hpp"
	"mesh_loader.cpp"
)