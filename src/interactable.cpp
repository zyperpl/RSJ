#include "interactable.hpp"

#include "asteroid.hpp"
#include "dialog.hpp"
#include "game.hpp"
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
  GAME.play_action(Action::Type::ChangeLevel, Level::Station);
}

DialogEntity::DialogEntity(const Vector2 &position, const std::string &name)
  : Interactable{}
  , dialogs{ Dialog::load_dialogs(name) }
{
  sprite = Sprite{ "resources/npc.aseprite", "idle_down" };
  sprite.set_centered();
  sprite.position = position;
}

void DialogEntity::update() {}

void DialogEntity::interact()
{
  set_dialog_id(Dialog::START_DIALOG_ID);
  GAME.play_action(Action::Type::Dialog, *this);
}
