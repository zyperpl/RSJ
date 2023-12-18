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
  if (IsKeyReleased(KEY_DOWN))
  {
    sprite.set_tag("idle_down");
  }
  else if (IsKeyReleased(KEY_UP))
  {
    sprite.set_tag("idle_up");
  }
  else if (IsKeyReleased(KEY_LEFT))
  {
    sprite.set_tag("idle_left");
  }
  else if (IsKeyReleased(KEY_RIGHT))
  {
    sprite.set_tag("idle_right");
  }

  if (IsKeyDown(KEY_DOWN))
  {
    velocity.y = 2.0f;
    sprite.set_tag("walk_down");
  }
  else if (IsKeyDown(KEY_UP))
  {
    velocity.y = -2.0f;
    sprite.set_tag("walk_up");
  }
  else
  {
    velocity.y = 0.0f;
  }

  if (IsKeyDown(KEY_LEFT))
  {
    velocity.x = -2.0f;
    sprite.set_tag("walk_left");
  }
  else if (IsKeyDown(KEY_RIGHT))
  {
    velocity.x = 2.0f;
    sprite.set_tag("walk_right");
  }
  else
  {
    velocity.x = 0.0f;
  }
}

void PlayerCharacter::update()
{
  handle_input();

  position.x += velocity.x;
  position.y += velocity.y;

  sprite.animate();
  mask.position = position;
}

void PlayerCharacter::die()
{
}
