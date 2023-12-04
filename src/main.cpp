#include <cassert>
#include <cmath>
#include <memory>

#include <raylib.h>
#include <rlgl.h>
#include <GLFW/glfw3.h>

#include "game.hpp"
#include "game_object.hpp"
#include "renderer.hpp"

#if defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>
#endif

std::unique_ptr<Game> game;
std::unique_ptr<Renderer> renderer;

static float accumulator = 0.0f;

void update_draw_frame()
{
  assert(game);
  assert(renderer);

  const float screen_width_float  = static_cast<float>(GetScreenWidth());
  const float screen_height_float = static_cast<float>(GetScreenHeight());

  float scale =
    std::max(std::min(screen_width_float / (float)(game_width), screen_height_float / (float)(game_height)), 1.0f);
  if (game->config.integer_scaling)
    scale = std::floor(scale);

  Rectangle render_destination;
  render_destination.x      = (screen_width_float - (game_width * scale)) * 0.5f;
  render_destination.y      = (screen_height_float - (game_height * scale)) * 0.5f;
  render_destination.width  = game_width * scale;
  render_destination.height = game_height * scale;

  const float interval = Game::delta_time;
  size_t steps         = 6;
  const float dt       = GetFrameTime();
  accumulator += dt;

  game->input().update();
  bool updated = false;
  while (accumulator >= interval && !game->quit)
  {
    game->update_all();
    updated = true;

    accumulator -= interval;

    steps--;
    if (steps == 0)
    {
      accumulator = 0;
    }
  }
  if (updated)
    game->input().reset();

  BeginDrawing();
  {
    renderer->render_game_objects(*game);

    renderer->render_game();
    renderer->render_menu(*game);
    renderer->render_screen(render_destination);

    DrawPoly(Vector2 { game_width / 2.0f, game_height / 2.0f }, 6, 10.0f, 0.0f, PURPLE);
  }
  EndDrawing();
  glFinish();
}

int main(void)
{
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(window_width, window_height, "GAME: THE GAME");
  SetWindowState(FLAG_VSYNC_HINT);
  SetExitKey(KEY_NULL);
  InitAudioDevice();
  SetTargetFPS(60);

  renderer = std::make_unique<Renderer>();
  game     = std::make_unique<Game>();

#if defined(EMSCRIPTEN)
  emscripten_set_main_loop(update_draw_frame, 0, 1);
#else
  while (!WindowShouldClose())
  {
    update_draw_frame();

    if (game->quit)
      CloseWindow();
  }
#endif
  game.reset();

  CloseAudioDevice();

  if (IsWindowReady())
    CloseWindow();

  return 0;
}
