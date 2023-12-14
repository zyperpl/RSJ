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
  uint8_t size{ 2 };
  uint8_t max_life { 1 };
  uint8_t life{ max_life };
  mutable Sprite sprite{ "resources/asteroid.aseprite" };
  Mask mask{};

  [[nodiscard]] static Asteroid create(const Vector2 &position, uint8_t size);

  bool update();
  void die();

  void draw() const noexcept;

private:
  Asteroid() = default;

  DECLARE_FRIEND_OBJECT_CIRCULAR_BUFFER()
};
