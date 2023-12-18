#pragma once

#include <array>
#include <memory>

#include <raylib.h>
#include <raymath.h>

#define CONFIG(Option) Game::config.Option
#define GAME           Game::get()

class Player;
class Bullet;
class Asteroid;
class Particle;
class Pickable;

template<typename T, size_t>
struct ObjectCircularBuffer;

struct Config
{
  bool show_debug{ false };
  bool show_masks{ false };
  bool show_velocity{ false };
  bool debug_bullets{ false };
};

enum class GameState
{
  MENU,
  PLAYING_PAUSED,
  PLAYING_ASTEROIDS,
  PLAYING_STATION,
  GAME_OVER
};

class Game
{
public:
  static Game &get() noexcept;

  std::unique_ptr<Player> player;
  std::unique_ptr<ObjectCircularBuffer<Bullet, 128>> bullets;
  std::unique_ptr<ObjectCircularBuffer<Asteroid, 1024>> asteroids;
  std::unique_ptr<ObjectCircularBuffer<Particle, 4096>> particles;
  std::unique_ptr<ObjectCircularBuffer<Pickable, 1024>> pickables;

  static constexpr int width               = 640;
  static constexpr int height              = 360;
  static constexpr float delta_time        = 1.0f / 60.0f;
  static constexpr int NUMBER_OF_ASTEROIDS = 10;
  static Config config;
  static uint64_t frame;

  void init();
  void update();
  void draw() noexcept;

  size_t coins{ 0 };
  size_t score{ 0 };

  GameState state { GameState::MENU };

private:
  [[nodiscard]] Game() noexcept = default;

  std::array<Vector2, 100> stars;
  std::unique_ptr<Pickable> station;
  void update_background() noexcept;
  void draw_background() noexcept;
  void set_state(GameState new_state) noexcept;
};
