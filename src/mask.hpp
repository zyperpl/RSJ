#pragma once

#include <variant>
#include <vector>

#include <raylib.h>
#include <raymath.h>

struct Circle
{
  Vector2 center{};
  float radius{};
};

typedef std::variant<Circle, Rectangle> Shape;

struct Mask
{
  Mask() noexcept = default;
  Mask(const Vector2 &position, const Shape &shape) noexcept;
  Mask(const Shape &shape) noexcept;
  Mask(const std::vector<Shape> &shapes) noexcept;
  Mask(const Mask &other) noexcept = default;
  Mask(Mask &&other) noexcept      = default;

  Mask &operator=(const Mask &other) noexcept = default;
  Mask &operator=(Mask &&other) noexcept      = default;

  Vector2 position{};
  std::vector<Shape> shapes;

  [[nodiscard]] bool check_collision(const Mask &other, float inflate = 0.0f) const;
  void draw() const noexcept;
};
