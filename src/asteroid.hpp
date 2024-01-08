#pragma once

#include "raylib.h"
#include "raymath.h"

#include "mask.hpp"
#include "object_circular_buffer.hpp"
#include "sound_manager.hpp"
#include "utils.hpp"

class Asteroid
{
public:
  enum class Type : uint8_t
  {
    Size1 = 0,
    Size2,
    Size3,
    Crystal,
    AlienShip,
    AlienBullet
  };

  Vector2 position{};
  Vector2 velocity{};
  Type type{ Type::Size3 };
  uint8_t max_life : 4 { 1 };
  uint8_t life : 4 { max_life };
  Mask mask{};

  [[nodiscard]] static Asteroid create_normal(const Vector2 &position, uint8_t size);
  [[nodiscard]] static Asteroid create_crystal(const Vector2 &position);
  [[nodiscard]] static Asteroid create_alien_ship(const Vector2 &position);
  [[nodiscard]] static Asteroid create_alien_bullet(const Vector2 &position, const Vector2 &direction);

  bool update();

  void draw() const noexcept;

  uint8_t size() const noexcept;

private:
  Asteroid() = default;
  void die();

  DECLARE_FRIEND_OBJECT_CIRCULAR_BUFFER()

  static std::unique_ptr<Sprite> ASTEROID_SPRITE;
  static std::unique_ptr<Sprite> ALIEN_SHIP_SPRITE;

  friend class Game;
};
