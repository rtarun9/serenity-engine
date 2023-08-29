#include "serenity-engine/core/file_system.hpp"

namespace serenity::core
{
    void FileSystem::init()
    {
        // Logic : Start from the currrent directory, and keep moving up until you can find the directory
        // called "serenity-engine". Then, that path + "serenity-engine" will be the root directory.

        auto current_path = std::filesystem::current_path();

        while (current_path.has_parent_path())
        {
            if (std::filesystem::is_directory(current_path / "serenity-engine"))
            {
                s_root_directory = current_path.string() + "/serenity-engine/"s;
                Log::info(std::format("located root directory {}", s_root_directory));
                break;
            }

            current_path = current_path.parent_path();
        }

        if (s_root_directory.empty())
        {
            Log::critical(std::format("could not locate root directory"));
        }
    }
} // namespace serenity::core