#include "quest.hpp"

#include "game.hpp"

SMSound Quest::sound_complete;

void Quest::report() noexcept
{
  if (!sound_complete.is_loaded())
    sound_complete = SoundManager::get("resources/tada.wav");

  sound_complete.play();

  if (!reported)
  {
    GAME.score += score;

    if (on_report)
      on_report();

    GAME.gui->show_message("Quest completed!");
    reported = true;
  }
}
