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
  GAME.schedule_action_change_level(Level::Station, 0, this);
}

DockedShip::DockedShip()
{
  sprite = Sprite{ "resources/docked_ship.aseprite" };
  sprite.set_centered();
  sprite.position = Vector2{ Game::width * 0.5f, Game::height * 0.25f };
}

void DockedShip::interact()
{
  GAME.schedule_action_ship_control(this);
}

DialogEntity::DialogEntity(const Vector2 &position, const std::string &name)
  : Interactable{}
  , dialogs{ Dialog::load_dialogs(name) }
{
  sprite = Sprite{ "resources/npc.aseprite", "idle_down" };
  sprite.set_centered();
  sprite.position = position;

  start_position = position;
}

void DialogEntity::update()
{
  if (!wander)
  {
    if (!sprite.is_playing_animation(default_animation_tag))
      sprite.set_animation(default_animation_tag);
  }

  if (wander)
  {
    auto &position = sprite.position;

    if (wander_timer.is_done())
    {
      velocity = Vector2Zero();

      if (GetRandomValue(0, 1) == 0)
      {
        const Direction dir    = static_cast<Direction>(GetRandomValue(0, 3));
        const float walk_speed = 0.5f;
        switch (dir)
        {
          case Direction::Up:
            velocity.y = -walk_speed;
            break;
          case Direction::Down:
            velocity.y = walk_speed;
            break;
          case Direction::Left:
            velocity.x = -walk_speed;
            break;
          case Direction::Right:
            velocity.x = walk_speed;
            break;
        }

        if (Vector2Distance(start_position, position) > 50.0f)
        {
          if (position.x < start_position.x + 10.0f)
            velocity.x = 1.0f;
          else if (position.x > start_position.x - 10.0f)
            velocity.x = -1.0f;
          else
            velocity.x = 0.0f;

          if (position.y < start_position.y - 10.0f)
            velocity.y = 1.0f;
          else if (position.y > start_position.y + 10.0f)
            velocity.y = -1.0f;
          else
            velocity.y = 0.0f;
        }

        velocity = Vector2Normalize(velocity);
      }

      wander_timer.set_max_time(static_cast<float>(GetRandomValue(2, 5)));
      wander_timer.start();
    }

    position.x += velocity.x;
    position.y += velocity.y;

    // TODO: check collisions

    wander_timer.update();
  }

  if (velocity.x < 0.0f)
    direction = Direction::Left;
  else if (velocity.x > 0.0f)
    direction = Direction::Right;
  else if (velocity.y < 0.0f)
    direction = Direction::Up;
  else if (velocity.y > 0.0f)
    direction = Direction::Down;

  if (fabs(velocity.x) > 0.0f || fabs(velocity.y) > 0.0f)
    sprite.set_animation(walk_tag_from_direction(direction));
  else
    sprite.set_animation(idle_tag_from_direction(direction));

  sprite.animate();
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

void Blocker::update()
{
  if (condition_quest_name.empty() || sprite.position.x < 0.0f)
    return;

  const auto &quest = QUEST(condition_quest_name);
  if (quest.is_completed())
  {
    sprite.scale      = Vector2Zero();
    sprite.position.x = -1000.0f;
    return;
  }
}
