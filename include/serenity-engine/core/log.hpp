#pragma once

#include <spdlog/fwd.h>

namespace serenity::core
{
    // A static class for logging purposes. Logs to console and file.
    // As its a static class, ensure to call the init and destroy functions before doing any logging.
    // Uses spdlog internally.
    class Log
    {
      public:
        static void init();
        static void destroy();

        static void info(const std::string_view message);
        static void warn(const std::string_view message);

        static void error(const std::string_view message,
                          const std::source_location source_location = std::source_location::current());
        static void critical(const std::string_view message,
                             const std::source_location source_location = std::source_location::current());

      private:
        explicit Log() = delete;

        static std::string format_source_location(const std::source_location &source_location);

      private:
        static inline std::shared_ptr<spdlog::logger> s_logger{};
    };
} // namespace serenity::core