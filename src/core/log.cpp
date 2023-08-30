#include "serenity-engine/core/log.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace serenity::core
{
    Log::Log()
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
        m_logger = std::make_shared<spdlog::logger>("Logger", sinks.begin(), sinks.end());
        m_logger->set_level(spdlog::level::info);

        info("Created logger");
    }

    Log::~Log()
    {
        info("Destroyed logger");
        spdlog::shutdown();
    }

    void Log::info(const std::string_view message)
    {
        m_logger->info(message);
    }

    void Log::warn(const std::string_view message)
    {
        m_logger->warn(message);
    }
     
    void Log::error(const std::string_view message, const std::source_location source_location)
    {
        m_logger->error(std::string(message) + format_source_location(source_location));
    }

    void Log::critical(const std::string_view message, const std::source_location source_location)
    {
        const auto critical_message = std::string(message) + format_source_location(source_location);
        m_logger->critical(critical_message);

        throw std::runtime_error(critical_message);
    }

    std::string Log::format_source_location(const std::source_location &source_location) const
    {
        return std::format("\n[file : {}.\nfunction : {}.\nline : {}.]", source_location.file_name(),
                           source_location.function_name(), source_location.line());
    }
} // namespace serenity::core