#include "game.hpp"

#include <algorithm>

#include "animation_sprite.hpp"
#include "game_object.hpp"
#include "sound_manager.hpp"

float Game::delta_time = 1.0f / 60.0f;

std::vector<std::shared_ptr<GameObject>> Game::objects;

GameObjectCollection<128> Game::collectibles;
GameObjectCollection<4096> Game::particles;

Game::Game()
{
#if defined(EMSCRIPTEN)
  config.shader_quality = 2.0f;
#endif

#if defined(_WIN32)
  SetWindowState(FLAG_VSYNC_HINT);
  set_fps_limit(120);
#endif

  objects.reserve(128);
}

Game::~Game()
{
  objects.clear();
  collectibles.clear();
  particles.clear();

  SoundManager::clear();
}

void Game::input_update()
{
  input_manager.update();
}

void Game::update_all()
{
  SoundManager::volume = config.volume_sound;
}

void Game::input_reset()
{
  input_manager.reset();
}

void Game::draw_objects(const RenderPass &render_pass) {}

void Game::set_fps_limit(int new_fps)
{
  config.fps = new_fps;
  SetTargetFPS(new_fps);
}
