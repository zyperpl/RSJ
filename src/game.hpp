#pragma once

#include <memory>

#include <raylib.h>
#include <raymath.h>

class Player;
class Bullet;
class Asteroid;

template<typename T>
class ObjectCircularBuffer;

class Game
{
public:
  static Game &get() noexcept;

  std::unique_ptr<Player> player;
  std::unique_ptr<ObjectCircularBuffer<Bullet>> bullets;
  std::unique_ptr<ObjectCircularBuffer<Asteroid>> asteroids;

  static constexpr int width               = 960;
  static constexpr int height              = 540;
  static constexpr float delta_time        = 1.0f / 60.0f;
  static constexpr int NUMBER_OF_ASTEROIDS = 10;

  void init();

  void update();

  void draw() noexcept;

private:
  Camera3D camera { };
};
