#include "utils.hpp"

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <cmath>
#include <string>

#include "game.hpp"

Rectangle texture_rect(const Texture2D &texture)
{
  return Rectangle{ 0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height) };
}

Rectangle texture_rect_flipped(const Texture2D &texture)
{
  return Rectangle{
    static_cast<float>(texture.width), 0.0f, static_cast<float>(texture.width), static_cast<float>(-texture.height)
  };
}

void wrap_position(Vector2 &position)
{
  if (position.x < 0)
    position.x = Game::width - 1;

  if (position.x >= Game::width)
    position.x = 0;

  if (position.y < 0)
    position.y = Game::height - 1;

  if (position.y >= Game::height)
    position.y = 0;
}
