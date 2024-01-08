#pragma once

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <unordered_map>
#include <variant>
#include <vector>

#include <raylib.h>
#include <raymath.h>

#include "dialog.hpp"
#include "gui.hpp"
#include "quest.hpp"
#include "room.hpp"

#define CONFIG(Option)       Game::config.Option
#define GAME                 Game::get()
#define QUEST(quest_name)    Game::get().quests.at(quest_name)
#define MISSION(mission_num) Game::get().missions.at(mission_num)

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
  bool show_fps{ false };
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

struct Artifact
{
  std::string name;
  std::string description;
};

enum class GunType
{
  Normal,
  Fast,
  Assisted,
  Homing
};
struct MissionParameters
{
  std::string name{ "Mission" };
  std::string description{};
  size_t number_of_asteroids{ 1 };
  size_t number_of_asteroid_crystals{ 0 };
  float survive_time_seconds{ 0 };

  void unlock() noexcept;
  [[nodiscard]] bool is_unlocked() const noexcept { return unlocked; }

  bool unlocked{ false };
};

class Game
{
public:
  [[nodiscard]] static Game &get() noexcept;

  std::unique_ptr<Player> player;
  std::shared_ptr<Room> room;
  std::unique_ptr<Sprite> tileset_sprite;
  std::unique_ptr<ObjectCircularBuffer<Bullet, 128>> bullets;
  std::unique_ptr<ObjectCircularBuffer<Asteroid, 2048>> asteroids;
  std::unique_ptr<ObjectCircularBuffer<Particle, 4096>> particles;
  std::unique_ptr<ObjectCircularBuffer<Pickable, 1024>> pickables;

  static constexpr int width               = 480;
  static constexpr int height              = 270;
  static constexpr int NUMBER_OF_ASTEROIDS = 6;
  static Config config;
  static uint64_t frame;

  void init();
  void unload() noexcept;
  void update();
  void draw() noexcept;

  std::unique_ptr<GUI> gui;

  size_t crystals{ 0 };
  size_t score{ 0 };
  std::queue<Artifact> artifacts;
  std::unordered_map<GunType, bool> guns{ { GunType::Normal, true },
                                          { GunType::Fast, false },
                                          { GunType::Assisted, false },
                                          { GunType::Homing, false } };
  GunType gun{ GunType::Normal };

  GameState get_state() const noexcept { return state; }

  void schedule_action_change_level(const Level &, size_t mission, const Interactable *) noexcept;
  void schedule_action_change_room(const Room::Type &) noexcept;
  void schedule_action_conversation(DialogEntity &) noexcept;
  void schedule_action_shop(const Interactable *) noexcept;
  void schedule_action_modify_ship(const Interactable *) noexcept;
  void schedule_action_ship_control(const Interactable *) noexcept;
  void schedule_action_mission_select(const Interactable *) noexcept;

  void set_room(const Room::Type &) noexcept;
  void set_mission(size_t mission) noexcept;

  size_t current_mission{ 0 };

  bool freeze_entities{ false };

  std::unordered_map<std::string, Quest> quests;

  std::map<size_t, MissionParameters> missions;

  float survive_time { 0.0f };
private:
  [[nodiscard]] Game() noexcept = default;

  Game(const Game &)            = delete;
  Game(Game &&)                 = delete;
  Game &operator=(const Game &) = delete;
  Game &operator=(Game &&)      = delete;

  ~Game() noexcept;

  void update_game();

  Camera2D camera;

  std::array<Vector2, 100> stars;
  std::unique_ptr<Sprite> asteroid_bg_sprite;
  void update_background() noexcept;
  void draw_background() noexcept;

  GameState state{ GameState::MENU };
  void set_state(GameState new_state) noexcept;

  std::queue<Action> actions;

  friend class GUI;
};
