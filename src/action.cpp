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

void Game::schedule_action_change_level(const Level &level, const Interactable *obj) noexcept
{
  TraceLog(LOG_INFO, "Changing level to %i", static_cast<int>(level));

  // player animation
  {
    Action action;
    action.on_update = [this, target_rect = obj->get_sprite().get_destination_rect()](Action &action)
    {
      bool reached_interactable{ false };
      const Vector2 target_position{ target_rect.x, target_rect.y };
      if (PlayerShip *player_ship = dynamic_cast<PlayerShip *>(player.get()); player_ship)
      {
        player_ship->position = Vector2Lerp(player_ship->position, target_position, 0.05f);

        if (player_ship->sprite.scale.x > 0.1f)
          player_ship->sprite.scale = Vector2Scale(player_ship->sprite.scale, 0.9f);

        reached_interactable =
          Vector2Distance(player_ship->sprite.position, target_position) < 4.0f || player_ship->sprite.scale.x < 0.1f;
      }
      if (PlayerCharacter *player_character = dynamic_cast<PlayerCharacter *>(player.get()); player_character)
      {
        const auto &pos = player_character->position;
        const float walk_speed = 0.5f;
        if (pos.x < target_position.x)
          player_character->velocity.x = walk_speed;
        else if (pos.x > target_position.x)
          player_character->velocity.x = -walk_speed;
        else
          player_character->velocity.x = 0.0f;

        if (pos.y < target_position.y)
          player_character->velocity.y = walk_speed;
        else if (pos.y > target_position.y)
          player_character->velocity.y = -walk_speed;
        else
          player_character->velocity.y = 0.0f;

        player_character->position = Vector2Add(player_character->position, player_character->velocity);
        player_character->animate();

        if (Vector2Distance(player_character->position, target_position) < 8.0f)
          reached_interactable = true;
      }

      freeze_entities = true;

      if (reached_interactable)
        action.is_done = true;
    };

    actions.push(std::move(action));
  }

  // fade-out transition
  {
    Action action;
    action.on_update = [](Action &action)
    {
      action.data = std::get<float>(action.data) + Game::delta_time * TRANSITION_SPEED;

      if (std::get<float>(action.data) >= 1.0f)
        action.is_done = true;
    };

    action.on_draw = [](const Action &action)
    {
      const float &data = std::get<float>(action.data);
      const float size  = std::max(width, height) * 0.5f * data;
      DrawPoly(Vector2{ width * 0.5f, height * 0.5f }, 8, size, data * 0.1f, BLACK);
    };

    action.on_done = [this, level](Action &)
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

  // fade-in transition
  {
    Action action;
    action.on_update = [](Action &action)
    {
      action.data = std::get<float>(action.data) + Game::delta_time * TRANSITION_SPEED;

      if (std::get<float>(action.data) >= 1.0f)
        action.is_done = true;
    };

    action.on_draw = [](const Action &action)
    {
      const float &data = std::get<float>(action.data);
      DrawRing(Vector2{ width * 0.5f, height * 0.5f }, width * data, width, 0.0f, 360.0f, 8, BLACK);
    };

    action.on_done = [this](Action &) { freeze_entities = false; };

    action.data = 0.0f;
    actions.push(std::move(action));
  }
}

void Game::schedule_action_conversation(DialogEntity &entity) noexcept
{
  Action action;

  action.on_start = [this, &entity](Action &action)
  {
    TraceLog(LOG_INFO, "Playing dialog %p(%s)", (void *)(&entity), entity.get_dialog_id().c_str());

    freeze_entities = true;

    dialog = entity.dialog();
    if (dialog->actor_name == Dialog::END_DIALOG_ID)
    {
      TraceLog(LOG_INFO, "Dialog ended");
      action.is_done = true;
      return;
    }

    if (!dialog->responses.empty())
      selected_dialog_response_index = 0;
  };

  action.on_update = [this](Action &action)
  {
    if (!dialog->responses.empty() && !selected_dialog_response_index.has_value())
      selected_dialog_response_index.value() = 0;

    if (selected_dialog_response_index.has_value())
    {
      size_t &response_index = selected_dialog_response_index.value();

      if (IsKeyPressed(KEY_DOWN))
      {
        response_index++;
        if (response_index >= dialog->responses.size())
          response_index = 0;
      }

      if (IsKeyPressed(KEY_UP))
      {
        if (response_index == 0)
          response_index = dialog->responses.size() - 1;
        else
          response_index--;
      }
    }

    if (IsKeyPressed(KEY_SPACE) || IsKeyPressedRepeat(KEY_SPACE))
    {
      if (selected_dialog_response_index.has_value())
      {
        TraceLog(LOG_TRACE, "Selected dialog response: %zu", selected_dialog_response_index.value());
        assert(selected_dialog_response_index.value() < dialog->responses.size());
        const auto &response       = dialog->responses[selected_dialog_response_index.value()];
        const auto &next_dialog_id = response.next_dialog_id;

        if (response.func)
          response.func();

        if (next_dialog_id.starts_with('_'))
        {
          if (next_dialog_id == "_end")
            TraceLog(LOG_INFO, "Dialog ended");
          else
            TraceLog(LOG_WARNING, "Unknown dialog id: %s", next_dialog_id.c_str());
        }
        else
        {
          action.data = DialogId{ next_dialog_id };
        }
      }

      action.is_done = true;
    }
  };

  action.on_done = [this, &entity](Action &action)
  {
    dialog.reset();
    selected_dialog_response_index.reset();

    if (std::holds_alternative<DialogId>(action.data))
    {
      const auto &next_dialog_id = std::get<DialogId>(action.data);
      entity.set_dialog_id(next_dialog_id);
      schedule_action_conversation(entity);
    }
    freeze_entities = false;
  };

  actions.push(std::move(action));
}
