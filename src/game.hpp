#pragma once

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <queue>
#include <variant>
#include <vector>

#include <raylib.h>
#include <raymath.h>

#include "dialog.hpp"
#include "mask.hpp"
#include "utils.hpp"

#define CONFIG(Option) Game::config.Option
#define GAME           Game::get()

class Sprite;
class Player;
class Bullet;
class Asteroid;
class Particle;
class Pickable;
class Interactable;
class DialogEntity;
struct Mask;

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

struct Action
{
  enum class Type
  {
    Invalid,
    ChangeLevel,
    Dialog,
  };

  std::variant<float, DialogId> data{};
  std::function<void(Action &action)> on_start{};
  std::function<void(Action &action)> on_update{};
  std::function<void(const Action &action)> on_draw{};
  std::function<void(Action &action)> on_done{};
  bool is_done{ false };
  bool has_started{ false };

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

  void start()
  {
    if (on_start)
    {
      on_start(*this);
      has_started = true;
    }
  }

  void done()
  {
    if (on_done)
      on_done(*this);
  }
};

class Game
{
public:
  [[nodiscard]] static Game &get() noexcept;

  std::unique_ptr<Player> player;
  std::unique_ptr<ObjectCircularBuffer<Bullet, 128>> bullets;
  std::unique_ptr<ObjectCircularBuffer<Asteroid, 2048>> asteroids;
  std::unique_ptr<ObjectCircularBuffer<Particle, 4096>> particles;
  std::unique_ptr<ObjectCircularBuffer<Pickable, 1024>> pickables;
  std::vector<std::unique_ptr<Interactable>> interactables;
  std::vector<Mask> masks;

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
  void play_action(const Action::Type &action_type, const Level &) noexcept;
  void play_action(const Action::Type &action_type, DialogEntity &) noexcept;

  bool freeze_entities{ false };

  std::optional<Dialog> dialog;
  std::optional<size_t> selected_dialog_response_index;

  Font font;
  Font dialog_font;

private:
  [[nodiscard]] Game() noexcept = default;

  Game(const Game &)            = delete;
  Game(Game &&)                 = delete;
  Game &operator=(const Game &) = delete;
  Game &operator=(Game &&)      = delete;

  void update_game();

  std::array<Vector2, 100> stars;
  void update_background() noexcept;
  void draw_background() noexcept;

  GameState state{ GameState::MENU };
  void set_state(GameState new_state) noexcept;

  std::queue<Action> actions;
};
