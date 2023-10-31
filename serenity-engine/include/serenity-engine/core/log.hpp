#pragma once

#include "singleton_instance.hpp"

#include <spdlog/fwd.h>

namespace serenity::core
{
    // A singleton class for logging purposes. Logs to console and file. Uses spdlog internally.
    // note(rtarun9) : Instance of log will be created by engine, no need to manually define it.
    // The SingletonInstance<> provides a instance() method, which will be used to access logging - related functions of
    class Log final : public SingletonInstance<Log>
    {
      public:
        explicit Log(const bool enable_console_log = true, const bool enable_file_log = true);
        ~Log();

        void info(const std::string_view message);
        void warn(const std::string_view message);

        void error(const std::string_view message,
                   const std::source_location source_location = std::source_location::current());
        void critical(const std::string_view message,
                      const std::source_location source_location = std::source_location::current());

        void add_sink(const std::shared_ptr<spdlog::sinks::sink> &sink, const std::string_view sink_name);
        void delete_sink(const std::string_view sink_name);

      private:
        std::string format_source_location(const std::source_location &source_location) const;

      private:
        Log(const Log &other) = delete;
        Log &operator=(const Log &other) = delete;

        Log(Log &&other) = delete;
        Log &operator=(Log &&other) = delete;

      private:
        std::shared_ptr<spdlog::logger> m_logger{};

        // A hashmap of sinks and the corresponding name.
        // Sinks added via the add_sink function will be stored here to allow for easy deletion of sinks (based on
        // usage).
        std::unordered_map<std::string, std::shared_ptr<spdlog::sinks::sink>> m_external_sinks{};
    };
} // namespace serenity::core