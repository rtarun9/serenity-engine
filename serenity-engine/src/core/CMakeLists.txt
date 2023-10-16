target_sources(serenity-engine PUBLIC
	"${SERENITY_ENGINE_INCLUDE_PATH}/core/singleton_instance.hpp"
	"${SERENITY_ENGINE_INCLUDE_PATH}/core/input.hpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/core/application.hpp"
	"application.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/core/file_system.hpp"
	"file_system.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/core/log.hpp"
	"log.cpp"
)