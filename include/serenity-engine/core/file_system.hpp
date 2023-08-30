#pragma once

#include "singleton_instance.hpp"

namespace serenity::core
{
    // A singleton class primarily used to get root source directory / paths relative to the executable.
    // NOTE : Instance of log will be created by engine, no need to manually define it.
    // The SingletonInstance<> provides a get() method, which will be used to access file system - related functions of
    // this class.
    class FileSystem : public SingletonInstance<FileSystem>
    {
      public:
        explicit FileSystem();
        ~FileSystem() = default;

        std::string get_root_directory()
        {
            return s_root_directory;
        }

        std::string get_relative_path(const std::string_view path)
        {
            return s_root_directory + "/"s + std::string(path);
        }

      private:
        FileSystem(const FileSystem &other) = delete;
        FileSystem &operator=(const FileSystem &other) = delete;
        FileSystem(FileSystem &&other) = delete;
        FileSystem &operator=(FileSystem &&other) = delete;

      private:
        static inline std::string s_root_directory{};
    };
} // namespace serenity::core