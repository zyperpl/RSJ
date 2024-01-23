#define _USE_MATH_DEFINES
#include <cmath>
#include <functional>
#include <memory>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#if defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>
#endif

#include "game.hpp"
#include "player.hpp"
#include "render_pass.hpp"
#include "utils.hpp"

const constexpr int AUDIO_BUFFER_SIZE   = (4096 * 12);
const constexpr size_t MAX_UPDATE_STEPS = 3;

const constexpr int window_width  = Game::width * 2;
const constexpr int window_height = Game::height * 2;

const constexpr bool integer_scaling = false;

static std::unique_ptr<RenderPass> game_render_pass;
static std::unique_ptr<RenderPass> ui_render_pass;

void update_draw_frame()
{
  const float screen_width_float  = static_cast<float>(GetScreenWidth());
  const float screen_height_float = static_cast<float>(GetScreenHeight());

  Game &game = Game::get();

  float scale = std::min(screen_width_float / (float)(Game::width), screen_height_float / (float)(Game::height));
  if (integer_scaling)
    scale = std::floor(scale);

  Rectangle render_destination;
  render_destination.x      = (screen_width_float - (Game::width * scale)) * 0.5f;
  render_destination.y      = (screen_height_float - (Game::height * scale)) * 0.5f;
  render_destination.width  = Game::width * scale;
  render_destination.height = Game::height * scale;

  const float dt       = GetFrameTime();
  const float interval = DELTA_TIME;
  const float fps      = 1.0f / dt;

  static float accumulator = 0.0f;
  accumulator += dt;

  game.input.gather(); // only sets the input state, does not unset it
  bool updated = false;
  for (size_t steps = 0; accumulator >= interval && steps < MAX_UPDATE_STEPS; ++steps)
  {
    accumulator -= interval;

    game.update();

    updated = true;

    if (--steps == 0)
      break;

    game.input.update();
  }

  BeginDrawing();
  {
    if (updated)
    {
      game_render_pass->render();
      ui_render_pass->render();
    }

    ClearBackground(BLACK);
    game_render_pass->draw(render_destination);
    ui_render_pass->draw(render_destination);

#if defined(DEBUG)
    game.input.debug_draw();

    DrawText(TextFormat("FPS: %4.0f", fps), 40, 20, 10, GOLD);
    DrawText(TextFormat(" DT: %8.8f", dt), 40, 30, 10, GOLD);
#endif
  }
  EndDrawing();

  static float previous_time = GetTime();
  const float current_time   = GetTime();
  const float elapsed_time   = current_time - previous_time;
  previous_time              = current_time;

  const float target_time = 1.0f / 60.0f;
  if (elapsed_time < target_time)
  {
    const float wait_time = (target_time - elapsed_time);
    if (wait_time > 0.0f)
      WaitTime(wait_time);
  }
}

int main(void)
{
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(window_width, window_height, "SPACE SOMETHING");
  SetExitKey(KEY_NULL);

  SetAudioStreamBufferSizeDefault(AUDIO_BUFFER_SIZE);
  InitAudioDevice();
  SetTargetFPS(60);

#if defined(DEBUG)
  SetTraceLogLevel(LOG_TRACE);
#endif

  Game &game = Game::get();
  game.init();

  game_render_pass = std::make_unique<RenderPass>(Game::width, Game::height);
  ui_render_pass   = std::make_unique<RenderPass>(Game::width, Game::height);

  game_render_pass->render_func = [&game]()
  {
    ClearBackground(BLACK);
    game.draw();
  };

  ui_render_pass->render_func = [&game]() { game.gui->draw(); };

#if defined(EMSCRIPTEN)
  emscripten_set_main_loop(update_draw_frame, 0, 1);
#else
  while (!WindowShouldClose())
  {
    update_draw_frame();

    if (IsKeyDown(KEY_ESCAPE))
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
