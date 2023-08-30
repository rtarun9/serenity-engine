#include "serenity-engine/core/application.hpp"

class Game final : public serenity::core::Application
{
  public:
    Game() = default;
    ~Game() = default;

  private:
};

// note(rtarun9) : Make a proper 'entry point' later.
int main()
{
    try
    {
        Game game{};
        game.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}