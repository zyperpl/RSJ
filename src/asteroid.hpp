#pragma once

#include "raylib.h"
#include "raymath.h"

#include "mask.hpp"
#include "object_circular_buffer.hpp"
#include "utils.hpp"

class Asteroid
{
public:
  Vector2 position{};
  Vector2 velocity{};
  int size{ 2 };
  mutable Sprite sprite{ "resources/asteroid.aseprite" };
  Mask mask{};

  [[nodiscard]] static Asteroid create(const Vector2 &position, int size);

  ObjectState update();
  void die();

  void draw() const noexcept;
};
