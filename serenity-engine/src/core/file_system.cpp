#include "serenity-engine/core/file_system.hpp"

#include "serenity-engine/core/log.hpp"

namespace serenity::core
{
    FileSystem::FileSystem()
    {
        // Logic : Start from the current directory, and keep moving up until you can find the directory
        // called "data". Then, that path + "data" will be the root directory.

        auto current_path = std::filesystem::current_path();
        Log::instance().info(std::format("Executable path : {}", current_path.string()));

        while (current_path.has_parent_path() && current_path != current_path.parent_path())
        {
            if (std::filesystem::is_directory(current_path / "data"))
            {
                m_root_directory = current_path.string() + "/"s;

                Log::instance().info(std::format("Located root directory {}", m_root_directory));

                break;
            }

            current_path = current_path.parent_path();
        }

        if (m_root_directory.empty())
        {
            Log::instance().critical(
                std::format("Could not locate root directory. Do you have a data folder in project directory?"));
        }
    }

    std::string FileSystem::read_file(const std::string_view path) const
    {
        auto file = std::ifstream(std::string(path));
        if (!file.is_open())
        {
            core::Log::instance().warn(std::format("Failed to open file with path : {}", path));
            return {};
        }

        auto file_contents = std::vector<std::string>();

        auto stream_buffer = std::stringstream{};
        stream_buffer << file.rdbuf();

        return stream_buffer.str();
    }

    void FileSystem::write_to_file(const std::string_view path, const std::string_view buffer) const
    {
        auto file = std::ofstream(std::string(path));
        if (!file.is_open())
        {
            core::Log::instance().warn(std::format("Failed to open file with path : {}", path));
            return;
        }

        file << buffer.data();

        file.close();
    }
} // namespace serenity::core