#include "serenity-engine/core/file_system.hpp"

int main()
{
    using namespace serenity;

    try
    {
        auto logger = std::make_unique<core::Log>();

        core::Log::get().info("This is a info!");

        auto fs = std::make_unique<core::FileSystem>();
        core::FileSystem::get().get_root_directory();

        while (true)
        {
            core::Log::get().info("Hello!");
            core::Log::get().warn("Hello!");
            core::Log::get().error("Hello!");

            core::FileSystem::get().get_relative_path("//");
        }
    }
    catch (const std::exception &e)
    {
        if (core::Log::exists())
        {
            core::Log::get().critical(e.what());
        }
        else
        {
            std::cerr << e.what();
        }
    }
}