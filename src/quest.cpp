#include "quest.hpp"

#include "game.hpp"

void Quest::report() noexcept
{
  if (!reported)
    GAME.score += score;

  if (on_report)
    on_report();

  reported = true;
}
