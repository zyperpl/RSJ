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
  mask.shapes.push_back(Rectangle{ 0.0f, 0.0f, 16.0f, 32.0f });
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
    DrawCircle(static_cast<int>(interactable->get_sprite().position.x),
               static_cast<int>(interactable->get_sprite().position.y),
               5.0f,
               RED);
  }
}

void PlayerCharacter::handle_input()
{
  if (IsKeyDown(KEY_LEFT))
    velocity.x = -PLAYER_SPEED * 1.6f;
  else if (IsKeyDown(KEY_RIGHT))
    velocity.x = PLAYER_SPEED * 1.6f;
  else
    velocity.x = 0.0f;

  if (IsKeyDown(KEY_UP))
    velocity.y = -PLAYER_SPEED;
  else if (IsKeyDown(KEY_DOWN))
    velocity.y = PLAYER_SPEED;
  else
    velocity.y = 0.0f;

  velocity = Vector2Scale(Vector2Normalize(velocity), PLAYER_SPEED);

  const bool can_interact = interactable;
  if (IsKeyPressed(KEY_SPACE) && can_interact)
  {
    interactable->interact();
  }
}

std::string idle_tag_from_direction(Direction direction)
{
  switch (direction)
  {
    case Direction::Left:
      return "idle_left";
    case Direction::Right:
      return "idle_right";
    case Direction::Up:
      return "idle_up";
    case Direction::Down:
      return "idle_down";
  }
  return "idle_down";
}

std::string walk_tag_from_direction(Direction direction)
{
  switch (direction)
  {
    case Direction::Left:
      return "walk_left";
    case Direction::Right:
      return "walk_right";
    case Direction::Up:
      return "walk_up";
    case Direction::Down:
      return "walk_down";
  }
  return "walk_down";
}

bool PlayerCharacter::is_colliding() const
{
  for (const auto &obj : GAME.interactables)
  {
    if (mask.check_collision(Mask(obj->get_sprite().get_destination_rect())))
      return true;
  }

  for (const auto &mask : GAME.masks)
  {
    if (mask.check_collision(this->mask))
      return true;
  }

  return false;
}

void PlayerCharacter::update()
{
  handle_input();

  // animation
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
  for (const auto &obj : GAME.interactables)
  {
    const Mask obj_mask(obj->get_sprite().get_destination_rect());
    if (mask.check_collision(obj_mask, 2.0f))
    {
      interactable = obj.get();
      break;
    }
  }
}

void PlayerCharacter::die() {}
