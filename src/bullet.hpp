#pragma once

#include "raylib.h"
#include "raymath.h"

#include "object_circular_buffer.hpp"
#include "utils.hpp"

enum class BulletType : uint8_t
{
  Normal,
  Assisted,
  Homing
};

class Bullet
{
public:
  static Bullet create_normal(const Vector2 &position, const Vector2 &velocity);
  static Bullet create_assisted(const Vector2 &position, const Vector2 &velocity);
  static Bullet create_homing(const Vector2 &position, const Vector2 &velocity);

  Vector2 position{};
  Vector2 velocity{};
  Vector2 direction{};
  uint8_t life{ 1 };
  BulletType type{ BulletType::Normal };
  Vector2 get_target_position() const noexcept;

  bool update();
  void draw() const noexcept;

private:
  Bullet() = default;

  const Vector2 *target{ nullptr };

  DECLARE_FRIEND_OBJECT_CIRCULAR_BUFFER()
};
