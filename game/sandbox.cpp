#include "serenity-engine/core/application.hpp"

class Game final : public serenity::core::Application
{
  public:
    explicit Game() = default;
    ~Game() = default;

  private:
};

std::unique_ptr<serenity::core::Application> serenity::core::create_application()
{
    return std::make_unique<Game>();
}