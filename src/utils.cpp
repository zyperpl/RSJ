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

void draw_wrapped(const Rectangle &rect, const std::function<void(const Vector2 &)> draw_function)
{
  const auto &x = rect.x;
  const auto &y = rect.y;
  const auto &w = rect.width;
  const auto &h = rect.height;

  if (x <= w)
    draw_function(Vector2{ x + Game::width, y });

  if (x + w >= Game::width)
    draw_function(Vector2{ x - Game::width, y });

  if (y <= h)
    draw_function(Vector2{ x, y + Game::height });

  if (y + h >= Game::height)
    draw_function(Vector2{ x, y - Game::height });

  draw_function(Vector2{ x, y });
}

std::string idle_tag_from_direction(const Direction &direction)
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

std::string walk_tag_from_direction(const Direction &direction)
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
