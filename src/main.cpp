#include <cassert>
#include <cmath>
#include <memory>

#include <raylib.h>
#include <rlgl.h>
#include <GLFW/glfw3.h>

#if defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>
#endif

static float accumulator = 0.0f;

const int game_width  = 320;
const int game_height = 240;

const int window_width  = game_width;
const int window_height = game_height;

const bool integer_scaling = true;

const float delta_time = 1.0f / 60.0f;

#include "sprite.hpp"

void update_draw_frame()
{
  static Sprite sprite("resources/test.aseprite", "idle");

  const float screen_width_float  = static_cast<float>(GetScreenWidth());
  const float screen_height_float = static_cast<float>(GetScreenHeight());

  float scale =
    std::max(std::min(screen_width_float / (float)(game_width), screen_height_float / (float)(game_height)), 1.0f);
  if (integer_scaling)
    scale = std::floor(scale);

  Rectangle render_destination;
  render_destination.x      = (screen_width_float - (game_width * scale)) * 0.5f;
  render_destination.y      = (screen_height_float - (game_height * scale)) * 0.5f;
  render_destination.width  = game_width * scale;
  render_destination.height = game_height * scale;

  const float interval = delta_time;
  size_t steps         = 6;
  const float dt       = GetFrameTime();
  accumulator += dt;

  while (accumulator >= interval)
  {
    accumulator -= interval;

    sprite.animate();

    if (IsKeyDown(KEY_SPACE))
      sprite.set_tag("walk");
    else
      sprite.set_tag("idle");

    steps--;
    if (steps == 0)
    {
      accumulator = 0;
    }
  }

  BeginDrawing();
  {
    ClearBackground(BLACK);

    DrawPoly(Vector2 { game_width / 2.0f, game_height / 2.0f }, 6, 10.0f, 0.0f, PURPLE);

    sprite.position = Vector2 { game_width / 2.0f, game_height / 2.0f };
    sprite.draw();
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

  CloseAudioDevice();

  if (IsWindowReady())
    CloseWindow();

  return 0;
}
