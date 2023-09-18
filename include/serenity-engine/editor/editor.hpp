#pragma once

#include "serenity-engine/core/singleton_instance.hpp"

#include "serenity-engine/window/window.hpp"

#include "serenity-engine/renderer/rhi/rhi.hpp"

namespace serenity::editor
{
    // The editor for serenity-engine.
    // note(rtarun9) : For now the editor is embedded within the engine, but the goal is to keep the editor as separate
    // from the engine as possible. This is because in the future it is likely that the editor and game become separate
    // executables.
    // When this game - editor separation occurs, the editor and game will most likely create a window and pass it to
    // the editor. For now, the editor's constructor has a window parameter, which will be the same as the engines.
    class Editor final : public core::SingletonInstance<Editor>
    {
      public:
        Editor(window::Window &window);
        ~Editor();

        // As the name suggests, call this function to render the editor in the engine window.
        void render();

      private:
        void scene_panel();
        void renderer_panel();

      private:
        std::string m_ini_path{};
    };
} // namespace serenity::editor