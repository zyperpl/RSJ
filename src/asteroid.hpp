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
  uint8_t max_life{ 1 };
  uint8_t life{ max_life };
  Mask mask{};

  [[nodiscard]] static Asteroid create(const Vector2 &position, uint8_t size);

  bool update();

  void draw() const noexcept;

private:
  Asteroid() = default;
  void die();

  DECLARE_FRIEND_OBJECT_CIRCULAR_BUFFER()

  static std::unique_ptr<Sprite> ASTEROID_SPRITE;

  friend class Game;
};
