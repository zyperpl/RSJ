#include "player_character.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#include <functional>

#include <raylib.h>
#include <raymath.h>

#include "asteroid.hpp"
#include "bullet.hpp"
#include "game.hpp"
#include "interactable.hpp"
#include "particle.hpp"
#include "utils.hpp"

PlayerCharacter::PlayerCharacter()
{
  mask.shapes.push_back(Rectangle{ 0.0f, 8.0f, 16.0f, 16.0f });
  mask.position = position;
}

void PlayerCharacter::draw() const noexcept
{
  sprite.position.x = std::round(position.x);
  sprite.position.y = std::round(position.y);
  sprite.set_centered();

  sprite.draw();

  if (CONFIG(show_masks))
    mask.draw();

  if (interactable)
  {
    // TODO: draw interactable icon in GUI
  }
}

void PlayerCharacter::handle_input()
{
  const Game &game = Game::get();

  if (game.input.left_held())
    velocity.x = -PLAYER_SPEED * 1.6f;
  else if (game.input.right_held())
    velocity.x = PLAYER_SPEED * 1.6f;
  else
    velocity.x = 0.0f;

  if (game.input.up_held())
    velocity.y = -PLAYER_SPEED;
  else if (game.input.down_held())
    velocity.y = PLAYER_SPEED;
  else
    velocity.y = 0.0f;

  velocity = Vector2Scale(Vector2Normalize(velocity), PLAYER_SPEED);

  if (game.input.action_pressed() && can_interact())
  {
    TraceLog(LOG_TRACE, "Interacting with %p", (void *)interactable);
    interactable->interact();
  }
}

bool PlayerCharacter::is_colliding() const
{
  for (const auto &obj : GAME.room->interactables)
  {
    if (mask.check_collision(Mask(obj->get_sprite().get_destination_rect())))
      return true;
  }

  for (const auto &mask : GAME.room->masks)
  {
    if (mask.check_collision(this->mask))
      return true;
  }

  return false;
}

void PlayerCharacter::animate()
{
  if (velocity.x < 0.0f)
    direction = Direction::Left;
  else if (velocity.x > 0.0f)
    direction = Direction::Right;
  else if (velocity.y < 0.0f)
    direction = Direction::Up;
  else if (velocity.y > 0.0f)
    direction = Direction::Down;

  if (fabs(velocity.x) > 0.0f || fabs(velocity.y) > 0.0f)
  {
    if (!sound_step.is_playing())
    {
      sound_step.set_volume(GetRandomValue(10, 100) / 100.0f);
      sound_step.play();
    }
    sprite.set_animation(walk_tag_from_direction(direction));
  }
  else
    sprite.set_animation(idle_tag_from_direction(direction));
  sprite.animate();
}

void PlayerCharacter::update()
{
  handle_input();

  // animation
  animate();

  // collisions
  {
    const float pos_x = std::round(position.x + velocity.x);
    mask.position.x   = pos_x;
    if (!is_colliding())
      position.x = pos_x;
    else
    {
      if (fabs(velocity.x) > 0.25f)
      {
        const float step  = velocity.x > 0.0f ? 1.0f : -1.0f;
        uint8_t max_steps = 250;
        while (fabs(position.x - pos_x) > 0.0f && max_steps-- > 0)
        {
          position.x += step;
          mask.position.x = position.x;
          if (is_colliding())
          {
            position.x -= step;
            mask.position.x = position.x;
            velocity.x      = 0.0f;
            break;
          }
        }
      }
    }

    const float pos_y = std::round(position.y + velocity.y);
    mask.position.y   = pos_y;
    if (!is_colliding())
      position.y = pos_y;
    else
    {
      if (fabs(velocity.y) > 0.25f)
      {
        const float step  = velocity.y > 0.0f ? 1.0f : -1.0f;
        uint8_t max_steps = 250;
        while (fabs(position.y - pos_y) > 0.0f && max_steps-- > 0)
        {
          position.y += step;
          mask.position.y = position.y;
          if (is_colliding())
          {
            position.y -= step;
            mask.position.y = position.y;
            velocity.y      = 0.0f;
            break;
          }
        }
      }
    }
    mask.position = position;
  }

  interactable = nullptr;
  for (const auto &obj : GAME.room->interactables)
  {
    if (!obj->is_interactable())
      continue;

    const Mask obj_mask(obj->get_sprite().get_destination_rect());
    if (mask.check_collision(obj_mask, 2.0f))
    {
      interactable = obj.get();
      break;
    }
  }
}

void PlayerCharacter::die() {}
