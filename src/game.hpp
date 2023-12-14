#pragma once

#include <array>
#include <memory>

#include <raylib.h>
#include <raymath.h>

class Player;
class Bullet;
class Asteroid;
class Particle;
class Pickable;

template<typename T, size_t>
class ObjectCircularBuffer;

struct Config
{
  bool show_debug{ false };
  bool show_masks{ false };
  bool show_velocity{ false };
  bool debug_bullets{ false };
};

class Game
{
public:
  static Game &get() noexcept;

  std::unique_ptr<Player> player;
  std::unique_ptr<ObjectCircularBuffer<Bullet, 64>> bullets;
  std::unique_ptr<ObjectCircularBuffer<Asteroid, 128>> asteroids;
  std::unique_ptr<ObjectCircularBuffer<Particle, 1024>> particles;
  std::unique_ptr<ObjectCircularBuffer<Pickable, 64>> pickables;

  static constexpr int width               = 640;
  static constexpr int height              = 360;
  static constexpr float delta_time        = 1.0f / 60.0f;
  static constexpr int NUMBER_OF_ASTEROIDS = 10;
  static Config CONFIG;
  static uint64_t frame;

  void init();

  void update();

  void draw() noexcept;

  size_t coins{ 0 };
  size_t score{ 0 };

private:
  [[nodiscard]] Game() noexcept = default;

  std::array<Vector2, 100> stars;
  void draw_background() noexcept;
};
