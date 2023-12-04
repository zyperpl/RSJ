#include "sprite.hpp"

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstring>
#include <utility>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"

#define CUTE_ASEPRITE_IMPLEMENTATION
#include <cute_aseprite.h>

#pragma GCC diagnostic pop
#pragma clang diagnostic pop

Sprite::Sprite(std::string_view file_path)
  : path{ file_path }
{
  if (path.ends_with(".aseprite"))
  {
    ase = cute_aseprite_load_from_file(path.data(), nullptr);
    assert(ase);
    if (!ase || ase->frame_count <= 0 || ase->w <= 0 || ase->h <= 0)
    {
      TraceLog(LOG_ERROR, "Cannot load %s\n", path.data());
      return;
    }

    Image image = GenImageColor(ase->w * ase->frame_count, ase->h, BLANK);

    for (int i = 0; i < ase->frame_count; i++)
    {
      ase_frame_t *frame = ase->frames + i;
      Rectangle dest = { static_cast<float>(i * ase->w), 0.0f, static_cast<float>(ase->w), static_cast<float>(ase->h) };
      Rectangle src  = { 0.0f, 0.0f, static_cast<float>(ase->w), static_cast<float>(ase->h) };
      Image frameImage = { .data    = frame->pixels,
                           .width   = ase->w,
                           .height  = ase->h,
                           .mipmaps = 1,
                           .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
      ImageDraw(&image, frameImage, src, dest, WHITE);
    }

    texture = LoadTextureFromImage(image);
    UnloadImage(image);
  }
  else
  {
    texture = LoadTexture(std::string(path).c_str());
  }
}

Sprite::~Sprite()
{
  UnloadTexture(texture);

  if (ase)
    cute_aseprite_free(ase);
}

size_t Sprite::get_width() const
{
  if (!ase)
    return texture.width;

  return ase->w;
}

size_t Sprite::get_height() const
{
  if (!ase)
    return texture.height;

  return ase->h;
}

void Sprite::draw() const noexcept
{
  const float h_flip = scale.x > 0.0f ? 1.0f : -1.0f;
  const float v_flip = scale.y > 0.0f ? 1.0f : -1.0f;

  Rectangle source = { 0.0f, 0.0f, h_flip * texture.width, v_flip * texture.height };
  Rectangle dest   = { std::roundf(position.x + offset.x),
                       std::roundf(position.y + offset.y),
                       (float)texture.width * scale.x,
                       (float)texture.height * scale.y };
  if (ase)
  {
    source = { 0.0f, 0.0f, h_flip * (float)ase->w, v_flip * (float)ase->h };
    dest   = { std::roundf(position.x + offset.x),
               std::roundf(position.y + offset.y),
               (float)ase->w * fabsf(scale.x),
               (float)ase->h * fabsf(scale.y) };
  }
  Vector2 origin = { 0.0f, 0.0f };
  DrawTexturePro(texture, source, dest, origin, rotation, tint);
}

SpriteSlice::SpriteSlice(std::string_view file_path, ::Rectangle cut_mask)
  : Sprite{ file_path }
  , mask{ cut_mask }
{
}

void SpriteSlice::draw() const noexcept
{
  const float h_flip = scale.x > 0.0f ? 1.0f : -1.0f;
  const float v_flip = scale.y > 0.0f ? 1.0f : -1.0f;

  const float w = mask.width;
  const float h = mask.height;

  const Rectangle source = { mask.x, mask.y, h_flip * w, v_flip * h };
  const Rectangle dest   = { position.x + offset.x, position.y + offset.y, w * scale.x, h * scale.y };
  const Vector2 origin   = { 0.0f, 0.0f };

  DrawTexturePro(texture, source, dest, origin, rotation, tint);
}
