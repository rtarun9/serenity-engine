#include "serenity-engine/core/file_system.hpp"

int main()
{
    using namespace serenity;

    core::Log::init();

    core::Log::info(std::format("This is a info message! 2 * 123 = {}", 2 * 123));
    core::Log::warn("This is a warn message!");
    core::Log::critical("No!");
    core::Log::error("This is a error message!");

    core::FileSystem::init();

    core::Log::destroy();
}