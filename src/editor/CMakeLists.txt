set(SERENITY_ENGINE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include/serenity-engine")

target_sources(serenity-engine PUBLIC
	"${SERENITY_ENGINE_INCLUDE_PATH}/editor/editor.hpp"
	"editor.cpp"
)