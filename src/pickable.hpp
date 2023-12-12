#pragma once

#include "mask.hpp"
#include "sprite.hpp"
#include "utils.hpp"

class Pickable
{
public:
  Pickable() {}
  Pickable(Sprite &&sprite, const Vector2 &position, std::function<void()> func);

  bool update();
  void draw() const;

  mutable Sprite sprite;
  mutable Mask mask{ Circle{ Vector2{ 0.0f, 0.0f }, 10.0f } };
  Vector2 position{ 0.0f, 0.0f };
  Vector2 velocity{ 0.0f, 0.0f };

private:
  std::function<void()> func;
};
