#define _USE_MATH_DEFINES
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
#include "particle.hpp"
#include "player.hpp"
#include "render_pass.hpp"
#include "utils.hpp"

static float accumulator = 0.0f;

const int window_width  = Game::width;
const int window_height = Game::height;

const bool integer_scaling = false;

static RenderPass *game_render_pass;
static RenderPass *ui_render_pass;

static Font font;

void update_draw_frame()
{
  Game &game = Game::get();
  if (!game_render_pass->render_func)
    game_render_pass->render_func = [&]()
    {
      ClearBackground(BLACK);
      game.draw();
    };

  if (!ui_render_pass->render_func)
    ui_render_pass->render_func = [&]()
    {
      const float font_size = 10.0f;
      Vector2 text_position{ 10.0f, 10.0f };
      DrawTextEx(font, TextFormat("FPS: %i", GetFPS()), text_position, font_size, 1.0f, WHITE);

      text_position.y += font_size + 5.0f;
      const char *lives_text = "Lives: ";
      DrawTextEx(font, lives_text, text_position, font_size, 1.0f, WHITE);
      Vector2 text_size = MeasureTextEx(font, lives_text, font_size, 1.0f);
      float x           = text_position.x + text_size.x + 5.0f;
      float y           = text_position.y + text_size.y * 0.5f;
      for (int i = 0; i < game.player->lives; i++)
      {
        DrawCircle(x + i * 20, y, 5, RED);
      }

      text_position.y += font_size + 5.0f;
      static uint64_t draw_score = 0;
      uint64_t score_step        = std::max<uint64_t>(1, (game.score - draw_score) / 20);
      if (draw_score < game.score)
        draw_score += score_step;
      if (draw_score > game.score)
        draw_score = game.score;
      DrawTextEx(font, TextFormat("Score: %i", draw_score), text_position, font_size, 1.0f, WHITE);

      text_position.y += font_size + 5.0f;
      const char *crystals_text = TextFormat("Crystals: %i", game.coins);
      DrawTextEx(font, crystals_text, text_position, font_size, 1.0f, WHITE);
      text_size = MeasureTextEx(font, crystals_text, font_size, 1.0f);
      static Sprite crystal_sprite{ "resources/ore.aseprite" };
      crystal_sprite.set_frame(0);
      crystal_sprite.scale = Vector2{ 0.4f, 0.4f };
      crystal_sprite.set_centered();
      crystal_sprite.position.x = text_position.x + text_size.x + 5.0f;
      crystal_sprite.position.y = text_position.y + text_size.y * 0.5f;
      crystal_sprite.draw();
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
      accumulator = 0;
  }

  BeginDrawing();
  {
    game_render_pass->render();
    ui_render_pass->render();

    ClearBackground(RAYWHITE);
    game_render_pass->draw(render_destination);
    ui_render_pass->draw(render_destination);
  }
  EndDrawing();
  glFinish();
}

int main(void)
{
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(window_width, window_height, "ASTEROIDS");
  SetExitKey(KEY_NULL);
  InitAudioDevice();
  SetTargetFPS(60);

  font       = LoadFontEx("resources/Kenney Mini Square.ttf", 10, nullptr, 0);
  Game &game = Game::get();
  game.init();

  game_render_pass = new RenderPass(Game::width, Game::height);
  ui_render_pass   = new RenderPass(Game::width, Game::height);

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

  delete game_render_pass;
  delete ui_render_pass;

  if (IsAudioDeviceReady())
    CloseAudioDevice();

  if (IsWindowReady())
    CloseWindow();

  return 0;
}
