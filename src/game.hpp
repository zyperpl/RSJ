#pragma once

#include <array>
#include <functional>
#include <memory>
#include <queue>
#include <variant>

#include <raylib.h>
#include <raymath.h>

#define CONFIG(Option) Game::config.Option
#define GAME           Game::get()

class Sprite;
class Player;
class Bullet;
class Asteroid;
class Particle;
class Pickable;
class Interactable;

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

enum class Level
{
  None,
  Asteroids,
  Station,
};

enum class Transition
{
  Circle
};

struct Action
{
  enum class Type
  {
    Invalid,
    ChangeLevel,
    Transition,
  };

  std::variant<float> data{};
  std::function<void(Action &action)> on_update{};
  std::function<void(const Action &action)> on_draw{};
  std::function<void()> on_done{};
  bool done{ false };

  void update()
  {
    if (on_update)
      on_update(*this);
  }

  void draw() const
  {
    if (on_draw)
      on_draw(*this);
  }
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
  std::vector<std::unique_ptr<Interactable>> interactables;

  static constexpr int width               = 480;
  static constexpr int height              = 270;
  static constexpr float delta_time        = 1.0f / 60.0f;
  static constexpr int NUMBER_OF_ASTEROIDS = 10;
  static Config config;
  static uint64_t frame;

  void init();
  void update();

  void draw() noexcept;

  size_t coins{ 0 };
  size_t score{ 0 };

  GameState get_state() const noexcept { return state; }
  void play_action(const Action::Type &action_type, const Level &level) noexcept;

  bool freeze_entities{ false };

private:
  [[nodiscard]] Game() noexcept = default;

  void update_game();

  std::array<Vector2, 100> stars;
  void update_background() noexcept;
  void draw_background() noexcept;

  GameState state{ GameState::MENU };
  void set_state(GameState new_state) noexcept;

  std::queue<Action> actions;
};
