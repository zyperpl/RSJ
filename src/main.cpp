#include <cassert>
#include <cmath>
#include <functional>
#include <memory>

#include <GLFW/glfw3.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#if defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>
#endif

#include "asteroid.hpp"
#include "bullet.hpp"
#include "game.hpp"
#include "object_circular_buffer.hpp"
#include "player.hpp"
#include "render_pass.hpp"
#include "utils.hpp"

static float accumulator = 0.0f;

const int window_width  = Game::width;
const int window_height = Game::height;

const bool integer_scaling = false;

void update_draw_frame()
{
  Game &game = Game::get();
  static RenderPass game_render_pass(Game::width, Game::height);
  if (!game_render_pass.render_func)
    game_render_pass.render_func = [&]()
    {
      ClearBackground(BLACK);
      game.draw();
    };

  static RenderPass ui_render_pass(Game::width, Game::height);
  if (!ui_render_pass.render_func)
    ui_render_pass.render_func = [&]()
    {
      const float font_size = 10.0f;
      DrawText(TextFormat("FPS: %i", GetFPS()), 10, 10, font_size, WHITE);
      DrawText(TextFormat("Bullets: %i", game.bullets->size()), 10, 30, font_size, WHITE);
      DrawText(TextFormat("Asteroids: %i", game.asteroids->size()), 10, 50, font_size, WHITE);
      DrawText("Lives: ", 10, 70, font_size, WHITE);
      Vector2 text_size = MeasureTextEx(GetFontDefault(), "Lives: ", font_size, 1.0f);
      const float x     = 15 + text_size.x;
      const float y     = 70 + text_size.y * 0.5f;
      for (int i = 0; i < game.player->lives; i++)
      {
        DrawCircle(x + i * 20, y, 5, RED);
      }
    };

  const float screen_width_float  = static_cast<float>(GetScreenWidth());
  const float screen_height_float = static_cast<float>(GetScreenHeight());

  float scale =
    std::max(std::min(screen_width_float / (float)(Game::width), screen_height_float / (float)(Game::height)), 1.0f);
  if (integer_scaling)
    scale = std::floor(scale);

  Rectangle render_destination;
  render_destination.x      = (screen_width_float - (Game::width * scale)) * 0.5f;
  render_destination.y      = (screen_height_float - (Game::height * scale)) * 0.5f;
  render_destination.width  = Game::width * scale;
  render_destination.height = Game::height * scale;

  const float interval = Game::delta_time;
  size_t steps         = 6;
  const float dt       = GetFrameTime();
  accumulator += dt;

  auto update = [&]() { game.update(); };

  while (accumulator >= interval)
  {
    accumulator -= interval;

    update();

    steps--;
    if (steps == 0)
    {
      accumulator = 0;
    }
  }

  static Camera2D camera{};
  camera.target   = { 0.0f, 0.0f };
  camera.target   = Vector2{ Game::width / 2.0f, Game::height / 2.0f };
  camera.offset   = Vector2{ Game::width / 2.0f, Game::height / 2.0f };
  camera.rotation = 0.0f;
  camera.zoom     = 1.0f;

  BeginDrawing();
  {
    game_render_pass.render();
    ui_render_pass.render();

    ClearBackground(RAYWHITE);
    game_render_pass.draw(render_destination);
    ui_render_pass.draw(render_destination);
  }
  EndDrawing();
  glFinish();
}

int main(void)
{
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(window_width, window_height, "ASTEROIDS");
  SetWindowState(FLAG_VSYNC_HINT);
  SetExitKey(KEY_NULL);
  InitAudioDevice();
  SetTargetFPS(60);

  Game &game = Game::get();
  game.init();

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
