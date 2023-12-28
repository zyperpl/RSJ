#include "interactable.hpp"

#include "asteroid.hpp"
#include "dialog.hpp"
#include "game.hpp"
#include "player_character.hpp"
#include "utils.hpp"

void Interactable::draw() const
{
  sprite.set_centered();
  sprite.draw();
}

Station::Station()
{
  sprite = Sprite{ "resources/station.aseprite", "idle" };
  sprite.set_centered();
  sprite.position = Vector2{ Game::width * 0.5f, Game::height * 0.5f };
  sprite.scale    = Vector2{ 0.01f, 0.01f };
  sprite.tint     = ColorBrightness(BLACK, 0.7f);
}

void Station::update()
{
  const auto state = GAME.get_state();
  if (state != GameState::PLAYING_ASTEROIDS)
    return;

  if (!GAME.asteroids->empty())
    return;

  const auto &frame = GAME.frame;
  sprite.position   = Vector2{ GAME.width / 2.0f, GAME.height / 2.0f + sin(frame * 0.001f) * 10.0f };
  if (sprite.scale.x < 1.0f)
    sprite.scale = Vector2Add(sprite.scale, Vector2{ 0.01f, 0.01f });
}

void Station::interact()
{
  GAME.schedule_action_change_level(Level::Station, this);
}

DockedShip::DockedShip()
{
  sprite = Sprite{ "resources/ship.aseprite", "idle" };
  sprite.set_centered();
  sprite.position = Vector2{ Game::width * 0.5f, Game::height * 0.25f };
}

void DockedShip::interact()
{
  GAME.schedule_action_change_level(Level::Asteroids, this);
}

DialogEntity::DialogEntity(const Vector2 &position, const std::string &name)
  : Interactable{}
  , dialogs{ Dialog::load_dialogs(name) }
{
  sprite = Sprite{ "resources/npc.aseprite", "idle_down" };
  sprite.set_centered();
  sprite.position = position;
}

void DialogEntity::update()
{
  if (!sprite.is_playing_animation(default_animation_tag))
    sprite.set_animation(default_animation_tag);
}

void DialogEntity::interact()
{
  const auto &player_position = GAME.player->position;
  const auto &position        = sprite.position;

  const auto &diff = Vector2Subtract(player_position, position);
  if (abs(diff.x) > abs(diff.y))
  {
    if (diff.x > 0)
      sprite.set_animation("idle_right");
    else
      sprite.set_animation("idle_left");
  }
  else
  {
    if (diff.y > 0)
      sprite.set_animation("idle_down");
    else
      sprite.set_animation("idle_up");
  }

  set_dialog_id(Dialog::START_DIALOG_ID);
  GAME.schedule_action_conversation(*this);
}
