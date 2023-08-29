#include "serenity-engine/core/log.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace serenity::core
{
    void Log::init()
    {
        // Create the sinks (a console sink and file sink).
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);
        console_sink->set_pattern("[%^%l%$] %v");

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/log.txt", true);
        file_sink->set_level(spdlog::level::trace);
        file_sink->set_pattern("%+");

        const auto sinks = std::vector<spdlog::sink_ptr>{console_sink, file_sink};

        // Create the logger.
        s_logger = std::make_shared<spdlog::logger>("Logger", sinks.begin(), sinks.end());
        s_logger->set_level(spdlog::level::info);
    }

    void Log::destroy()
    {
        spdlog::shutdown();
    }

    void Log::info(const std::string_view message)
    {
        s_logger->info(message);
    }

    void Log::warn(const std::string_view message)
    {
        s_logger->warn(message);
    }

    void Log::error(const std::string_view message, const std::source_location source_location)
    {
        s_logger->error(std::string(message) + format_source_location(source_location));
    }

    void Log::critical(const std::string_view message, const std::source_location source_location)
    {
        s_logger->critical(std::string(message) + format_source_location(source_location));
    }

    std::string Log::format_source_location(const std::source_location &source_location)
    {
        return std::format("\n[file : {}.\nfunction : {}.\nline : {}.]", source_location.file_name(),
                           source_location.function_name(), source_location.line());
    }
} // namespace serenity::core