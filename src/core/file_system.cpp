#include "serenity-engine/core/file_system.hpp"

#include "serenity-engine/core/log.hpp"

namespace serenity::core
{
    FileSystem::FileSystem()
    {
        // Logic : Start from the currrent directory, and keep moving up until you can find the directory
        // called "serenity-engine". Then, that path + "serenity-engine" will be the root directory.

        auto current_path = std::filesystem::current_path();

        while (current_path.has_parent_path())
        {
            if (std::filesystem::is_directory(current_path / "serenity-engine"))
            {
                m_root_directory = current_path.string() + "/serenity-engine/"s;

                Log::get().info(std::format("Located root directory {}", m_root_directory));

                break;
            }

            current_path = current_path.parent_path();
        }

        if (m_root_directory.empty())
        {
            Log::get().critical(std::format("Could not locate root directory"));
        }
    }
} // namespace serenity::core