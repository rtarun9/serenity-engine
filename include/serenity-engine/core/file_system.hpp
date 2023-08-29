#pragma once

namespace serenity::core
{
    // A static class primarily used to get root source directory / paths relative to the executable.
    // Ensure to call the init function from the application.
    class FileSystem
    {
      public:
        static void init();

        static std::string get_root_directory()
        {
            return s_root_directory;
        }

        static std::string get_relative_path(const std::string_view path)
        {
            return s_root_directory + "/"s + std::string(path);
        }

      private:
        explicit FileSystem() = delete;

      private:
        static inline std::string s_root_directory{};
    };
} // namespace serenity::core