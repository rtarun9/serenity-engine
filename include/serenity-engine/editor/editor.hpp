#pragma once

#include "serenity-engine/core/singleton_instance.hpp"

#include "serenity-engine/window/window.hpp"

#include "serenity-engine/renderer/rhi/rhi.hpp"

namespace serenity::editor
{
    enum class EditorLogLevel
    {
        Info,
        Warn,
        Error,
        Critical
    };

    struct EditorLogMessage
    {
        std::string message{};
        EditorLogLevel log_level{};
    };

    // Returned by the text_editor_window function. Caller functions can take actions based on return value.
    enum class TextEditorAction
    {
        Open,
        Close,
        Reload,
        Save
    };

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
        void scripts_panel();
        void log_panel();

        // Take in a file_path and provides option to edit file and save.
        TextEditorAction text_editor_window(const std::string_view file_path);

      private:
        std::string m_ini_path{};

      public:
        std::vector<EditorLogMessage> m_editor_log_messages{};
    };
} // namespace serenity::editor