#pragma once

#include <memory>
#include <unordered_set>
#include <vector>

#include "input_manager.hpp"
#include "renderer.hpp"

#include "game_object.hpp"
#include "game_object_collection.hpp"

#include <raylib.h>

const int window_width  = 1280;
const int window_height = 720;
const int game_width    = 400;
const int game_height   = 226;
const float game_ratio  = static_cast<float>(game_width) / static_cast<float>(game_height);

struct GameConfig
{
  float volume_music{ 0.5f };
  float volume_sound{ 0.9f };
  bool integer_scaling{ false };
  float shader_quality{ 1.0f };
  int fps{ 60 };
  bool show_fps{ false };
};

class Game
{
public:
  Game();
  ~Game();

  void input_update();
  void input_reset();
  InputManager &input() { return input_manager; }

  void update_all();
  void draw_objects(const RenderPass &render_pass);

  GameConfig config;

  bool quit{ false };

  std::unordered_set<std::string> completed_levels;

  void set_fps_limit(int new_fps);

  static float delta_time;
  static std::vector<std::shared_ptr<GameObject>> objects;
  static GameObjectCollection<128> collectibles;
  static GameObjectCollection<4096> particles;

private:
  enum
  {
    Menu,
    Playing,
    Pause,
    End
  } state = Menu;

  InputManager input_manager;
};
