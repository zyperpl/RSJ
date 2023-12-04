#include "renderer.hpp"

#include "game.hpp"

#include <raymath.h>

Renderer::Renderer()
{
  game_render_target = LoadRenderTexture(game_width, game_height);
  SetTextureFilter(game_render_target.texture, TEXTURE_FILTER_POINT);
  SetTextureWrap(game_render_target.texture, TEXTURE_WRAP_MIRROR_REPEAT);

  objects_render_target = LoadRenderTexture(game_width, game_height);
  SetTextureFilter(objects_render_target.texture, TEXTURE_FILTER_POINT);
  SetTextureWrap(objects_render_target.texture, TEXTURE_WRAP_MIRROR_REPEAT);

  menu_render_target = LoadRenderTexture(game_width, game_height);
  SetTextureFilter(menu_render_target.texture, TEXTURE_FILTER_POINT);

  render_source = { 0.0f,
                    0.0f,
                    static_cast<float>(game_render_target.texture.width),
                    static_cast<float>(game_render_target.texture.height) };
}

Renderer::~Renderer()
{
  UnloadRenderTexture(game_render_target);
  UnloadRenderTexture(menu_render_target);
  UnloadRenderTexture(objects_render_target);
}

void Renderer::render_game_objects(Game &game) const
{
  BeginTextureMode(objects_render_target);
  {
    ClearBackground(BLACK);
    game.draw_objects(RenderPass::Normal);
  }
  EndTextureMode();
}

void Renderer::render_menu(Game &game) const
{
  BeginTextureMode(menu_render_target);
  {
    ClearBackground(BLANK);
    //game.draw_menu();
  }
  EndTextureMode();
}

void Renderer::render_game() const
{
  BeginTextureMode(game_render_target);
  DrawTexture(objects_render_target.texture, 0, 0, WHITE);
  EndTextureMode();
}

void Renderer::render_screen(const Rectangle &render_destination) const
{
  ClearBackground(BLACK);
  DrawTexturePro(game_render_target.texture, render_source, render_destination, Vector2Zero(), 0.0f, WHITE);
  const Rectangle flipped_render_source{ render_source.x, render_source.y, render_source.width, -render_source.height };
  DrawTexturePro(menu_render_target.texture, flipped_render_source, render_destination, Vector2Zero(), 0.0f, WHITE);

#if defined(DEBUG)
#if defined(DEBUG_RENDER_TARGETS)
  const float debug_rectangle_divider = 12.0f;
  Rectangle debug_rectangle{
    0.0f, 0.0f, render_destination.width / debug_rectangle_divider, render_destination.height / debug_rectangle_divider
  };
  DrawTexturePro(shadow_render_target.texture, render_source, debug_rectangle, Vector2Zero(), 0.0f, WHITE);

  debug_rectangle.x += debug_rectangle.width;
  DrawTexturePro(blocker_render_target_h.texture, render_source, debug_rectangle, Vector2Zero(), 0.0f, WHITE);

  debug_rectangle.x += debug_rectangle.width;
  DrawTexturePro(blocker_render_target.texture, render_source, debug_rectangle, Vector2Zero(), 0.0f, WHITE);

  debug_rectangle.x += debug_rectangle.width;
  DrawTexturePro(bloom_render_target.texture, render_source, debug_rectangle, Vector2Zero(), 0.0f, WHITE);

  debug_rectangle.x += debug_rectangle.width;
  DrawTexturePro(bloom_final_render_target.texture, render_source, debug_rectangle, Vector2Zero(), 0.0f, WHITE);
#endif

  DrawFPS(render_destination.width - 100, 0);
#endif
}
