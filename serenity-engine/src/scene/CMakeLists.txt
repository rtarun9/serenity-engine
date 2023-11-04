target_sources(serenity-engine PUBLIC
	"${SERENITY_ENGINE_INCLUDE_PATH}/scene/game_object.hpp"
	"game_object.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/scene/scene.hpp"
	"scene.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/scene/scene_manager.hpp"
	"scene_manager.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/scene/camera.hpp"
	"camera.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/scene/lights.hpp"
	"lights.cpp"
)