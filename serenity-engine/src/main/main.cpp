#include "serenity-engine/core/application.hpp"

int main()
{
    try
    {
        auto application = serenity::core::create_application();
        application->run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}