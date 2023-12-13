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

  virtual ~Bullet() = default;
  Vector2 position{};
  Vector2 velocity{};
  Vector2 direction{};
  const Vector2 *target{ nullptr };
  uint8_t life{ 1 };
  BulletType type{ BulletType::Normal };

  virtual bool update();
  virtual void draw() const noexcept;

private:
  Bullet() = default;

  DECLARE_FRIEND_OBJECT_CIRCULAR_BUFFER()
};
