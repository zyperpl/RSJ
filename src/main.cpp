#define _USE_MATH_DEFINES
#include <cmath>
#include <functional>
#include <memory>

#include <GLFW/glfw3.h> // for glFinish
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#if defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>
#endif

#include "asteroid.hpp"
#include "bullet.hpp"
#include "game.hpp"
#include "gui.hpp"
#include "object_circular_buffer.hpp"
#include "particle.hpp"
#include "player.hpp"
#include "render_pass.hpp"
#include "utils.hpp"

const int window_width  = Game::width * 2;
const int window_height = Game::height * 2;

const bool integer_scaling = false;

static std::unique_ptr<RenderPass> game_render_pass;
static std::unique_ptr<RenderPass> ui_render_pass;

void update_draw_frame()
{
  const int screen_width          = GetScreenWidth();
  const int screen_height         = GetScreenHeight();
  const float screen_width_float  = static_cast<float>(screen_width);
  const float screen_height_float = static_cast<float>(screen_height);

  Game &game = Game::get();

  if (!game_render_pass->render_func)
    game_render_pass->render_func = [&game]()
    {
      ClearBackground(BLACK);
      game.draw();
    };

  if (!ui_render_pass->render_func)
    ui_render_pass->render_func = [&game]() { game.gui->draw(); };

  float scale =
    std::max(std::min(screen_width_float / (float)(Game::width), screen_height_float / (float)(Game::height)), 1.0f);
  if (integer_scaling)
    scale = std::floor(scale);

  Rectangle render_destination;
  render_destination.x      = (screen_width_float - (Game::width * scale)) * 0.5f;
  render_destination.y      = (screen_height_float - (Game::height * scale)) * 0.5f;
  render_destination.width  = Game::width * scale;
  render_destination.height = Game::height * scale;

  const float interval = DELTA_TIME;
  size_t steps         = 6;
  const float dt       = GetFrameTime();

  static float accumulator = 0.0f;
  accumulator += dt;

  auto update = [&]() { game.update(); };

  while (accumulator >= interval)
  {
    accumulator -= interval;

    update();

    steps--;
    if (steps == 0)
      accumulator = 0;
  }

  BeginDrawing();
  {
    game_render_pass->render();
    ui_render_pass->render();

    ClearBackground(BLACK);
    game_render_pass->draw(render_destination);
    ui_render_pass->draw(render_destination);
  }
  EndDrawing();
  glFinish();
}

int main(void)
{
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(window_width, window_height, "SPACE III");
  SetExitKey(KEY_NULL);
  InitAudioDevice();
  SetTargetFPS(60);

  SetTraceLogLevel(LOG_TRACE);

  Game &game = Game::get();
  game.init();

  game_render_pass = std::make_unique<RenderPass>(Game::width, Game::height);
  ui_render_pass   = std::make_unique<RenderPass>(Game::width, Game::height);

#if defined(EMSCRIPTEN)
  emscripten_set_main_loop(update_draw_frame, 0, 1);
#else
  while (!WindowShouldClose())
  {
    update_draw_frame();

    if (IsKeyPressed(KEY_ESCAPE))
      CloseWindow();
  }
#endif

  game_render_pass.reset();
  ui_render_pass.reset();

  game.unload();
  SoundManager::clear();

  if (IsAudioDeviceReady())
    CloseAudioDevice();

  if (IsWindowReady())
    CloseWindow();

  return 0;
}
