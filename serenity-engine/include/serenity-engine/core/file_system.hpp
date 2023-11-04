#pragma once

#include "singleton_instance.hpp"

#include "serenity-engine/utils/string_conversions.hpp"

namespace serenity::core
{
    // A singleton class primarily used to get root source directory / absolute paths (with respect to main partition,
    // such as C://).
    // note(rtarun9) : Instance of file system will be created by engine, no need to manually define it. The
    // SingletonInstance<> provides a instance() method, which will be used to access file system - related functions of
    // this class.

    class FileSystem final : public SingletonInstance<FileSystem>
    {
      public:
        explicit FileSystem();
        ~FileSystem() = default;

        std::string get_root_directory() const { return m_root_directory; }

        // Returns (string of) absolute path of given path (with respect to root directory).
        // If path is already absolute, simply return parameter.
        std::string get_absolute_path(const std::string_view path) const
        {
            if (std::filesystem::path(std::string(path)).is_absolute())
            {
                return std::string(path);
            }

            return m_root_directory + std::string(path);
        }

        // Returns (wstring of) absolute path of given path (with respect to root directory).
        // If path is already absolute, simply return parameter.
        std::wstring get_absolute_path(const std::wstring_view path) const
        {
            if (std::filesystem::path(std::wstring(path)).is_absolute())
            {
                return std::wstring(path);
            }

            return string_to_wstring(m_root_directory) + std::wstring(path);
        }

        std::string read_file(const std::string_view path) const;
        void write_to_file(const std::string_view path, const std::string_view buffer) const;

      private:
        FileSystem(const FileSystem &other) = delete;
        FileSystem &operator=(const FileSystem &other) = delete;

        FileSystem(FileSystem &&other) = delete;
        FileSystem &operator=(FileSystem &&other) = delete;

      private:
        std::string m_root_directory{};
    };
} // namespace serenity::core