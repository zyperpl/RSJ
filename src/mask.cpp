#include "mask.hpp"

#include <algorithm>
#include <cmath>
#include <variant>

#include <raylib.h>
#include <raymath.h>

Mask::Mask(const Vector2 &position, const Shape &shape) noexcept
{
  this->position = position;
  shapes.push_back(shape);
}

Mask::Mask(const Shape &shape) noexcept
{
  shapes.push_back(shape);
}

Mask::Mask(const std::vector<Shape> &shapes) noexcept
  : shapes(shapes)
{
}

bool Mask::check_collision(const Mask &other) const
{
  const auto get_transformed_circle = [](const Shape &shape, const Vector2 &position)
  {
    const auto &circle        = std::get<Circle>(shape);
    const auto &center_offset = circle.center;
    const auto &radius        = circle.radius;

    return Circle{ Vector2Add(position, center_offset), radius };
  };

  const auto get_transformed_rectangle = [](const Shape &shape, const Vector2 &position)
  {
    const auto &rectangle = std::get<Rectangle>(shape);
    const auto &x         = rectangle.x;
    const auto &y         = rectangle.y;
    const auto &w         = rectangle.width;
    const auto &h         = rectangle.height;

    return Rectangle{ position.x + x, position.y + y, w, h };
  };

  const auto &this_shapes  = shapes;
  const auto &other_shapes = other.shapes;

  for (const auto &this_shape : this_shapes)
  {
    for (const auto &other_shape : other_shapes)
    {
      if (std::holds_alternative<Circle>(this_shape) && std::holds_alternative<Circle>(other_shape))
      {
        const auto &this_circle  = get_transformed_circle(this_shape, position);
        const auto &other_circle = get_transformed_circle(other_shape, other.position);

        if (CheckCollisionCircles(this_circle.center, this_circle.radius, other_circle.center, other_circle.radius))
          return true;
      }
      else if (std::holds_alternative<Rectangle>(this_shape) && std::holds_alternative<Rectangle>(other_shape))
      {
        const auto &this_rectangle  = get_transformed_rectangle(this_shape, position);
        const auto &other_rectangle = get_transformed_rectangle(other_shape, other.position);

        if (CheckCollisionRecs(this_rectangle, other_rectangle))
          return true;
      }
      else if (std::holds_alternative<Circle>(this_shape) && std::holds_alternative<Rectangle>(other_shape))
      {
        const auto &this_circle     = get_transformed_circle(this_shape, position);
        const auto &other_rectangle = get_transformed_rectangle(other_shape, other.position);

        if (CheckCollisionCircleRec(this_circle.center, this_circle.radius, other_rectangle))
          return true;
      }
      else if (std::holds_alternative<Rectangle>(this_shape) && std::holds_alternative<Circle>(other_shape))
      {
        const auto &this_rectangle = get_transformed_rectangle(this_shape, position);
        const auto &other_circle   = get_transformed_circle(other_shape, other.position);

        if (CheckCollisionCircleRec(other_circle.center, other_circle.radius, this_rectangle))
          return true;
      }
    }
  }

  return false;
}

void Mask::draw() const noexcept
{
  // get random, but predictable color based on *this pointer (lower bits have more entropy)
  const auto color = [&]() -> Color
  {
    const auto pointer = reinterpret_cast<uintptr_t>(this);
    int64_t seed = static_cast<int64_t>(pointer);
    seed ^= seed << 2;
    seed ^= seed >> 3;

    const auto r       = static_cast<uint8_t>((seed & 0xF0) | 0x0F);
    const auto g       = static_cast<uint8_t>(((seed >> 8) & 0xF0) | 0x0F);
    const auto b       = static_cast<uint8_t>(((seed >> 16) & 0xF0) | 0x0F);
    return Color{ r, g, b, 255 };
  }();

  for (const auto &shape : shapes)
  {
    if (std::holds_alternative<Circle>(shape))
    {
      const auto &circle = std::get<Circle>(shape);
      const auto &center = Vector2Add(position, circle.center);

      DrawCircleLines(center.x, center.y, circle.radius, color);
      DrawCircleLines(center.x + Game::width, center.y, circle.radius, color);
      DrawCircleLines(center.x - Game::width, center.y, circle.radius, color);
      DrawCircleLines(center.x, center.y + Game::height, circle.radius, color);
    }
    else if (std::holds_alternative<Rectangle>(shape))
    {
      const auto &rectangle = std::get<Rectangle>(shape);
      auto x                = rectangle.x + position.x;
      auto y                = rectangle.y + position.y;
      const auto &w         = rectangle.width;
      const auto &h         = rectangle.height;

      DrawRectangleLines(x, y, w, h, color);
      DrawRectangleLines(x + Game::width, y, w, h, color);
      DrawRectangleLines(x - Game::width, y, w, h, color);
      DrawRectangleLines(x, y + Game::height, w, h, color);
    }
  }
}
