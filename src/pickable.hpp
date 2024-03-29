#pragma once

#include <functional>

#include "mask.hpp"
#include "sprite.hpp"
#include "utils.hpp"

class Player;

class Pickable
{
public:
  enum class Type : uint8_t
  {
    Other    = 0,
    Ore      = 1,
    Artifact = 2
  };

  static Pickable create(const Vector2 &position, const Vector2 &velocity, const std::function<void()> &func);
  static Pickable create_ore(const Vector2 &position, const Vector2 &velocity);
  static Pickable create_artifact(const Vector2 &position, const Vector2 &velocity);
  bool update();
  void draw() const;

  int8_t player_id : 4 { -1 };
  Type type : 4 { Type::Other };
  mutable Mask mask{ Circle{ Vector2{ 0.0f, 0.0f }, 24.0f } };
  Vector2 position{ 0.0f, 0.0f };
  Vector2 velocity{ 0.0f, 0.0f };

private:
  Pickable() = default;
  Pickable(const Vector2 &position, const std::function<void()> &func);
  std::function<void()> func;

  DECLARE_FRIEND_OBJECT_CIRCULAR_BUFFER()

  static std::unique_ptr<Sprite> ORE_SPRITE;

  friend class Game;
};
