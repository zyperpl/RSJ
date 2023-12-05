#include "sprite.hpp"

#include <cassert>
#include <chrono>
#include <cmath>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif

#if defined(__clang__) && !defined(__GNUC__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#endif

#define CUTE_ASEPRITE_IMPLEMENTATION
#include <cute_aseprite.h>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
#if defined(__clang__) && !defined(__GNUC__)
#pragma clang diagnostic pop
#endif

Sprite::Sprite(std::string_view file_path, std::string_view tag_name) noexcept
  : path{ file_path }
{
  if (path.ends_with(".aseprite"))
  {
    load_texture_with_animation();
    set_tag(tag_name);
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

void Sprite::load_texture_with_animation()
{
  ase = cute_aseprite_load_from_file(path.data(), nullptr);
  if (!ase || ase->frame_count <= 0 || ase->w <= 0 || ase->h <= 0)
  {
    TraceLog(LOG_ERROR, "Cannot load %s\n", path.data());
    return;
  }

  Image image{ GenImageColor(ase->w * ase->frame_count, ase->h, BLANK) };

  for (int i = 0; i < ase->frame_count; i++)
  {
    const ase_frame_t *frame{ ase->frames + i };
    const Rectangle dest{
      static_cast<float>(i * ase->w), 0.0f, static_cast<float>(ase->w), static_cast<float>(ase->h)
    };
    const Rectangle src{ 0.0f, 0.0f, static_cast<float>(ase->w), static_cast<float>(ase->h) };
    const Image frameImage{ .data    = frame->pixels,
                            .width   = ase->w,
                            .height  = ase->h,
                            .mipmaps = 1,
                            .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    ImageDraw(&image, frameImage, src, dest, WHITE);
  }

  texture = LoadTextureFromImage(image);
  UnloadImage(image);

  for (int i = 0; i < ase->tag_count; ++i)
  {
    const auto &atag = ase->tags[i];
    TraceLog(LOG_INFO,
             "AnimationTag \"%s\", from %d to %d (out of %d frames).\n",
             atag.name,
             atag.from_frame,
             atag.to_frame,
             ase->frame_count);
    tags.insert(std::make_pair(atag.name, AnimationTag{ atag.from_frame, atag.to_frame }));
  }
}

Texture2D &Sprite::get_texture()
{
  return texture;
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

  const float sprite_w = static_cast<float>(ase->w);
  const float sprite_h = static_cast<float>(ase->h);

  const Rectangle source = { (float)(frame_index)*sprite_w, 0.0f, h_flip * sprite_w, v_flip * sprite_h };
  const Rectangle dest   = { std::roundf(position.x + offset.x),
                             std::roundf(position.y + offset.y),
                             sprite_w * fabsf(scale.x),
                             sprite_h * fabsf(scale.y) };

  const Vector2 origin{ 0.0f, 0.0f };
  DrawTexturePro(texture, source, dest, origin, rotation, tint);
}

void Sprite::reset_animation()
{
  frame_index = tag.start_frame;
  frame_timer = 0;
}

void Sprite::set_frame(int frame)
{
  frame_index = frame;
  frame_timer = 0;
}

int Sprite::get_frame() const
{
  return frame_index;
}

int Sprite::get_frame_count() const
{
  if (!ase) [[unlikely]]
    return 0;
  return ase->frame_count;
}

bool Sprite::should_advance_frame()
{
  using namespace std::chrono;
  const int64_t time_since_epoch_ms =
    duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();

  const int64_t time_difference_ms = last_time_ms != time_since_epoch_ms ? (time_since_epoch_ms - last_time_ms) : 0;
  last_time_ms                     = time_since_epoch_ms;

  if (tag.end_frame == tag.start_frame)
  {
    frame_index = tag.start_frame;
    frame_timer = 0;
    return false;
  }

  if (ase->frame_count <= 1) [[unlikely]]
    return false;

  const int64_t ase_frame_array_index = frame_index;
  assert(ase_frame_array_index < ase->frame_count);
  assert(ase_frame_array_index >= 0);

  if (frame_index < 0)
  {
    frame_index = ase->frame_count > 0 ? ase->frame_count - 1 : 0;
    return false;
  }
  if (frame_index >= ase->frame_count)
  {
    frame_index = 0;
    return false;
  }
  const auto ase_frame = ase->frames[ase_frame_array_index];

  const int frame_duration = ase_frame.duration_milliseconds;
  frame_timer += time_difference_ms;

  if (frame_timer >= frame_duration)
  {
    frame_timer -= frame_duration;
    if (frame_timer >= frame_duration)
      frame_timer = 0;

    return true;
  }

  return false;
}

void Sprite::animate(int step)
{
  if (!ase)
    return;

  if (should_advance_frame())
  {
    frame_index += step;
    if (frame_index > tag.end_frame || frame_index >= ase->frame_count)
      frame_index = tag.start_frame;

    if (frame_index < tag.start_frame || frame_index < 0)
      frame_index = tag.end_frame;
  }
}

void Sprite::set_tag(std::string_view tag_name)
{
  if (!ase) [[unlikely]]
    return;

  if (!tag_name.empty())
  {
    auto tags_iterator = tags.find(tag_name);
    if (tags_iterator != tags.end())
    {
      if (tag != tags_iterator->second)
      {
        tag = tags_iterator->second;
        reset_animation();
      }
    }
  }
  else
  {
    if (tag != default_tag)
    {
      reset_animation();
      tag = default_tag;
    }
  }
}
