#pragma once

#include <memory>

#include <raylib.h>
#include <raymath.h>

class Player;
class Bullet;
class Asteroid;

template<typename T>
class ObjectCircularBuffer;

struct Config
{
  bool show_debug{ true };
  bool show_masks{ true };
  bool show_velocity{ true };
  bool show_acceleration{ true };
};

class Game
{
public:
  static Game &get() noexcept;

  std::unique_ptr<Player> player;
  std::unique_ptr<ObjectCircularBuffer<Bullet>> bullets;
  std::unique_ptr<ObjectCircularBuffer<Asteroid>> asteroids;

  static constexpr int width               = 640;
  static constexpr int height              = 360;
  static constexpr float delta_time        = 1.0f / 60.0f;
  static constexpr int NUMBER_OF_ASTEROIDS = 30;
  static Config CONFIG;

  void init();

  void update();

  void draw() noexcept;

private:
  [[nodiscard]] Game() noexcept = default;

  void draw_background() noexcept;
};
