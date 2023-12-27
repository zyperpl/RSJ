#include "game.hpp"

#include <cassert>

#include <raylib.h>
#include <raymath.h>

#include "asteroid.hpp"
#include "bullet.hpp"
#include "interactable.hpp"
#include "object_circular_buffer.hpp"
#include "particle.hpp"
#include "pickable.hpp"
#include "player_character.hpp"
#include "player_ship.hpp"
#include "utils.hpp"

static constexpr const float TRANSITION_SPEED{ 1.25f };

void Game::play_action(const Action::Type &action_type, const Level &level) noexcept
{
  assert(action_type != Action::Type::Invalid);
  assert(action_type == Action::Type::ChangeLevel);

  TraceLog(LOG_INFO, "Changing level to %i", static_cast<int>(level));

  // player animation
  {
    Action action;
    action.on_update = [this](Action &action)
    {
      const auto &station_position = Vector2{ width * 0.5f, height * 0.5f };

      PlayerShip *player_ship = dynamic_cast<PlayerShip *>(player.get());
      if (!player_ship)
        return;

      player_ship->position = Vector2Lerp(player_ship->position, station_position, 0.05f);

      if (player_ship->sprite.scale.x > 0.1f)
        player_ship->sprite.scale = Vector2Scale(player_ship->sprite.scale, 0.9f);

      const bool near_station =
        Vector2Distance(player_ship->sprite.position, station_position) < 4.0f || player_ship->sprite.scale.x < 0.1f;

      freeze_entities = true;

      if (near_station)
        action.done = true;
    };

    actions.push(std::move(action));
  }

  // fade-in transition
  {
    Action action;
    action.on_update = [](Action &action)
    {
      action.data = std::get<float>(action.data) + Game::delta_time * TRANSITION_SPEED;

      if (std::get<float>(action.data) >= 1.0f)
        action.done = true;
    };

    action.on_draw = [](const Action &action)
    {
      const float &data = std::get<float>(action.data);
      const float size  = std::max(width, height) * 0.5f * data;
      DrawPoly(Vector2{ width * 0.5f, height * 0.5f }, 8, size, data * 0.1f, BLACK);
    };

    action.on_done = [this, level]()
    {
      switch (level)
      {
        case Level::None:
          assert(false);
          break;
        case Level::Asteroids:
          set_state(GameState::PLAYING_ASTEROIDS);
          break;
        case Level::Station:
          set_state(GameState::PLAYING_STATION);
          break;
      }
    };

    action.data = 0.0f;
    actions.push(std::move(action));
  }

  // fade-out transition
  {
    Action action;
    action.on_update = [](Action &action)
    {
      action.data = std::get<float>(action.data) + Game::delta_time * TRANSITION_SPEED;

      if (std::get<float>(action.data) >= 1.0f)
        action.done = true;
    };

    action.on_draw = [](const Action &action)
    {
      const float &data = std::get<float>(action.data);
      DrawRing(Vector2{ width * 0.5f, height * 0.5f }, width * data, width, 0.0f, 360.0f, 8, BLACK);
    };

    action.on_done = [this]() { freeze_entities = false; };

    action.data = 0.0f;
    actions.push(std::move(action));
  }
}

void Game::play_action(const Action::Type &action_type, const DialogId &dialog_id) noexcept
{
  assert(action_type != Action::Type::Invalid);
  assert(action_type == Action::Type::Dialog);
  assert(!dialog_id.empty());

  TraceLog(LOG_INFO, "Playing dialog %s", dialog_id.c_str());

  if (action_type != Action::Type::Dialog)
    return;

  Action action;

  action.on_update = [this, dialog_id](Action &action)
  {
    dialog = dialog_manager.get_dialog(dialog_id);

    if (!dialog->responses.empty())
      selected_dialog_response_index = 0;

    action.done = true;
  };

  actions.push(std::move(action));
}
