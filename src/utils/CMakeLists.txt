set(SERENITY_ENGINE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include/serenity-engine")

target_sources(serenity-engine PUBLIC
	"${SERENITY_ENGINE_INCLUDE_PATH}/utils/enum_value.hpp"
	"${SERENITY_ENGINE_INCLUDE_PATH}/utils/string_conversions.hpp"
)