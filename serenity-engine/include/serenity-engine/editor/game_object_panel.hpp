#pragma once

#include "serenity-engine/scene/game_object.hpp"

namespace serenity::editor
{
    // Class responsible for displaying the game object panel / settings UI.
    // Very bare-bones for now, but will be filled as more editor related functionality is added.
    class GameObjectPanel
    {
      public:
        void render_panel_for_game_object(scene::GameObject &game_object);

      private:
    };
} // namespace serenity::editor