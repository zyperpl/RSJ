#include "render_pass.hpp"

#include <cassert>
#include <raylib.h>

#include "utils.hpp"

RenderPass::RenderPass(int width, int height)
{
  render_texture = LoadRenderTexture(width, height);
  SetTextureFilter(render_texture.texture, TEXTURE_FILTER_POINT);
  SetTextureWrap(render_texture.texture, TEXTURE_WRAP_REPEAT);
  assert(IsRenderTextureReady(render_texture));
}

RenderPass::~RenderPass()
{
  UnloadRenderTexture(render_texture);
}

void RenderPass::render()
{
  BeginTextureMode(render_texture);
  {
    ClearBackground(BLANK);
    render_func();
  }
  EndTextureMode();
}

void RenderPass::draw(const Rectangle &render_destination)
{
  DrawTexturePro(render_texture.texture,
                 texture_rect_flipped(render_texture.texture),
                 render_destination,
                 Vector2Zero(),
                 0.0f,
                 WHITE);
}
