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
  Game &game = Game::get();
  if (!game_render_pass->render_func)
    game_render_pass->render_func = [&game]()
    {
      ClearBackground(BLACK);
      game.draw();
    };

  if (!ui_render_pass->render_func)
    ui_render_pass->render_func = [&]()
    {
      const float font_size = 10.0f;
      Vector2 text_position{ 10.0f, 10.0f };
      DrawTextEx(GAME.font, TextFormat("FPS: %i", GetFPS()), text_position, font_size, 1.0f, WHITE);

      text_position.y += font_size + 5.0f;
      const char *lives_text = "Lives: ";
      DrawTextEx(GAME.font, lives_text, text_position, font_size, 1.0f, WHITE);
      Vector2 text_size = MeasureTextEx(GAME.font, lives_text, font_size, 1.0f);
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
      DrawTextEx(GAME.font, TextFormat("Score: %i", draw_score), text_position, font_size, 1.0f, WHITE);

      text_position.y += font_size + 5.0f;
      const char *crystals_text = TextFormat("Crystals: %i", game.coins);
      DrawTextEx(GAME.font, crystals_text, text_position, font_size, 1.0f, WHITE);
      text_size = MeasureTextEx(GAME.font, crystals_text, font_size, 1.0f);
      static Sprite crystal_sprite{ "resources/ore.aseprite" };
      crystal_sprite.set_frame(0);
      crystal_sprite.scale = Vector2{ 0.4f, 0.4f };
      crystal_sprite.set_centered();
      crystal_sprite.position.x = text_position.x + text_size.x + 5.0f;
      crystal_sprite.position.y = text_position.y + text_size.y * 0.5f;
      crystal_sprite.draw();

      if (game.dialog)
      {
        const unsigned char r               = 120 + (sin(GetTime() * 6.0f) * 0.5f + 0.5f) * 100;
        const unsigned char g               = 120 + (sin(GetTime() * 6.0f + 2.0f) * 0.5f + 0.5f) * 100;
        const Color selected_response_color = Color{ r, g, 255, 255 };
        const Dialog &dialog                = *game.dialog;
        const float dialog_width            = 300.0f;
        const float dialog_height           = 100.0f;
        const float dialog_x                = (Game::width - dialog_width) * 0.5f;
        const float dialog_y                = Game::height - dialog_height - 10.0f;
        DrawRectangle(dialog_x, dialog_y, dialog_width, dialog_height, Color{ 0, 0, 0, 200 });
        DrawRectangleLinesEx(Rectangle{ dialog_x, dialog_y, dialog_width, dialog_height }, 1, WHITE);

        // name
        {
          DrawTextEx(
            GAME.font, dialog.actor_name.c_str(), Vector2{ dialog_x + 10.0f, dialog_y + 10.0f }, font_size, 2.0f, RAYWHITE);
          DrawTextEx(GAME.font,
                     dialog.actor_name.c_str(),
                     Vector2{ dialog_x + 10.0f + 1.0f, dialog_y + 10.0f },
                     font_size,
                     2.0f,
                     RAYWHITE);
          auto name_size = MeasureTextEx(GAME.font, dialog.actor_name.c_str(), font_size, 2.0f);
          DrawLineEx(Vector2{ dialog_x + 10.0f, dialog_y + 10.0f + name_size.y },
                     Vector2{ dialog_x + 10.0f + name_size.x, dialog_y + 10.0f + name_size.y },
                     1.0f,
                     RAYWHITE);
        }

        // dialog text
        {
          SetTextLineSpacing(font_size);
          DrawTextEx(GAME.dialog_font,
                     dialog.text.c_str(),
                     Vector2{ dialog_x + 10.0f, dialog_y + 10.0f + font_size + 5.0f },
                     font_size,
                     1.0f,
                     WHITE);
        }
        const auto text_size = MeasureTextEx(GAME.dialog_font, dialog.text.c_str(), font_size, 1.0f);

        const float response_x = dialog_x + 20.0f + 5.0f;
        const float response_y = dialog_y + 10.0f + text_size.y + font_size * 2.0f;
        for (size_t i = 0; i < dialog.responses.size(); i++)
        {
          const DialogResponse &response = dialog.responses[i];
          DrawTextEx(GAME.dialog_font,
                     response.text.c_str(),
                     Vector2{ response_x, response_y + (font_size + 5.0f) * i },
                     font_size,
                     1.0f,
                     WHITE);

          if (game.selected_dialog_response_index.has_value() && game.selected_dialog_response_index.value() == i)
          {
            DrawTextEx(GAME.dialog_font,
                       response.text.c_str(),
                       Vector2{ response_x, response_y + (font_size + 5.0f) * i },
                       font_size,
                       1.0f,
                       selected_response_color);

            const float triangle_size = 10.0f;
            const float triangle_x    = dialog_x + 10.0f;
            const float triangle_y    = response_y + font_size * 0.5f + (font_size + 5.0f) * i;
            DrawTriangle(Vector2{ triangle_x, triangle_y - triangle_size * 0.5f },
                         Vector2{ triangle_x, triangle_y + triangle_size * 0.5f },
                         Vector2{ triangle_x + triangle_size, triangle_y },
                         selected_response_color);
          }
        }
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

  Game &game  = Game::get();
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

  if (IsAudioDeviceReady())
    CloseAudioDevice();

  if (IsWindowReady())
    CloseWindow();

  return 0;
}
