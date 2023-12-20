#include "player_character.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#include <functional>

#include <raylib.h>
#include <raymath.h>

#include "asteroid.hpp"
#include "bullet.hpp"
#include "game.hpp"
#include "particle.hpp"
#include "utils.hpp"

PlayerCharacter::PlayerCharacter()
{
  mask.shapes.push_back(Rectangle{ 0.0f, 0.0f, 24.0f, 32.0f });
  mask.position = position;

  sprite.set_centered();
  sprite.position = position;
}

void PlayerCharacter::draw() const noexcept
{
  sprite.set_centered();
  sprite.position = position;
  sprite.draw();
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

void PlayerCharacter::update()
{
  handle_input();

  position.x += velocity.x;
  position.y += velocity.y;

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
  mask.position = position;
}

void PlayerCharacter::die() {}
