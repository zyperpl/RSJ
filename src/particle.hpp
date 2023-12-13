#pragma once

#include <array>
#include <memory>

#include <raylib.h>
#include <raymath.h>

#include "object_circular_buffer.hpp"
#include "utils.hpp"

class Particle
{
public:
  static Particle create(const Vector2 &position, const Vector2 &velocity, const Color &color) noexcept;

  bool update() noexcept;
  void draw() const noexcept;

  Vector2 position{ 0.0f, 0.0f };
  Vector2 velocity{ 0.0f, 0.0f };
  Color color{ 255, 255, 255, 255 };

private:
  Particle() = default;

  DECLARE_FRIEND_OBJECT_CIRCULAR_BUFFER()
};
