#pragma once

#include <functional>

#include "mask.hpp"
#include "sprite.hpp"
#include "utils.hpp"

class Player;

class Pickable
{
public:
  static Pickable create(const Vector2 &position, const Vector2 &velocity, const std::function<void()> &func);
  static Pickable create_ore(const Vector2 &position, const Vector2 &velocity);
  bool update();
  void draw() const;

  int8_t player_id{ -1 };
  mutable Mask mask{ Circle{ Vector2{ 0.0f, 0.0f }, 24.0f } };
  Vector2 position{ 0.0f, 0.0f };
  Vector2 velocity{ 0.0f, 0.0f };

private:
  Pickable() = default;
  Pickable(const Vector2 &position, const std::function<void()> &func);
  std::function<void()> func;

  DECLARE_FRIEND_OBJECT_CIRCULAR_BUFFER()
};
