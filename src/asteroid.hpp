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
  int max_life { 2 };
  int life{ max_life };
  mutable Sprite sprite{ "resources/asteroid.aseprite" };
  Mask mask{};

  [[nodiscard]] static Asteroid create(const Vector2 &position, int size);

  bool update();
  void die();

  void draw() const noexcept;

private:
  Asteroid() = default;

  DECLARE_FRIEND_OBJECT_CIRCULAR_BUFFER()
};
