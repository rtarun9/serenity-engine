target_sources(serenity-engine PUBLIC
	"${SERENITY_ENGINE_INCLUDE_PATH}/editor/imgui_sink.hpp"
	
	"${SERENITY_ENGINE_INCLUDE_PATH}/editor/editor.hpp"
	"editor.cpp"

	"${SERENITY_ENGINE_INCLUDE_PATH}/editor/game_object_panel.hpp"
	"game_object_panel.cpp"
)