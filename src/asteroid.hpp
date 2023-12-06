#pragma once

#include "raylib.h"
#include "raymath.h"

#include "object_circular_buffer.hpp"
#include "utils.hpp"

class Asteroid
{
public:
  Vector2 position{};
  Vector2 velocity{};
  float rotation{ 0.0f };
  float rotation_speed{ 0.0f };
  float size{ 20.0f };

  [[nodiscard]] static Asteroid create(const Vector2 &position, float rotation, float size);

  ObjectState update();

  void draw() const noexcept;
};
