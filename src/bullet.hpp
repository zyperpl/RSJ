#pragma once

#include "raylib.h"
#include "raymath.h"

#include "object_circular_buffer.hpp"
#include "utils.hpp"

class Bullet
{
public:
  Vector2 position{};
  Vector2 velocity{};
  uint8_t life{ 30 };

  bool update();
  void draw() const noexcept;
};
