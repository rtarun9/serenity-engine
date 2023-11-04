#pragma once

#include "editor.hpp"

#include "imgui.h"
#include "spdlog/sinks/base_sink.h"

namespace serenity::editor
{
    // Custom spdlog sink for imgui.
    // Directly appends EditorLogMessages into the editor's message vector.
    // This approach (of having editor log messages) is inspired from https://github.com/skaarj1989/SupernovaEngine.
    template <typename Mutex>
    class ImGuiSink : public spdlog::sinks::base_sink<Mutex>
    {
      protected:
        void sink_it_(const spdlog::details::log_msg &message) override
        {
            const auto spdlog_level_to_editor_log_level = [](const spdlog::level::level_enum level) -> EditorLogLevel {
                switch (level)
                {

                case spdlog::level::info: {
                    return EditorLogLevel::Info;
                }
                break;

                case spdlog::level::warn: {
                    return EditorLogLevel::Warn;
                }
                break;

                case spdlog::level::err: {
                    return EditorLogLevel::Error;
                }
                break;

                case spdlog::level::critical: {

                    return EditorLogLevel::Critical;
                }
                break;

                default: {

                    return EditorLogLevel::Info;
                }
                break;
                }
            };

            auto formatted = spdlog::memory_buf_t{};
            spdlog::sinks::base_sink<Mutex>::formatter_->format(message, formatted);

            if (Editor::exists())
            {
                Editor::instance().m_editor_log_messages.emplace_back(EditorLogMessage{
                    .message = fmt::to_string(formatted),
                    .log_level = spdlog_level_to_editor_log_level(message.level),
                });
            }
        }

        void flush_() override {}
    };

} // namespace serenity::editor
