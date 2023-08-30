#pragma once

#include "singleton_instance.hpp"

#include <spdlog/fwd.h>

namespace serenity::core
{
    // A singleton class for logging purposes. Logs to console and file.
    // Uses spdlog internally.
    // NOTE : Instance of log will be created by engine, no need to manually define it.
    // The SingletonInstance<> provides a get() method, which will be used to access logging - related functions of this
    // class.
    // note(rtarun9) : Add option to disable logging to file in future.
    class Log final : public SingletonInstance<Log>
    {
      public:
        explicit Log();
        ~Log();

        void info(const std::string_view message);
        void warn(const std::string_view message);

        void error(const std::string_view message,
                   const std::source_location source_location = std::source_location::current());
        void critical(const std::string_view message,
                      const std::source_location source_location = std::source_location::current());

      private:
        std::string format_source_location(const std::source_location &source_location) const;

      private:
        Log(const Log &other) = delete;
        Log &operator=(const Log &other) = delete;

        Log(Log &&other) = delete;
        Log &operator=(Log &&other) = delete;

      private:
        std::shared_ptr<spdlog::logger> m_logger{};
    };
} // namespace serenity::core