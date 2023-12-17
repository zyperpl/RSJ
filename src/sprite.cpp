#include "sprite.hpp"

#define _USE_MATH_DEFINES
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

static std::unordered_map<std::string, Sprite> sprite_cache{};
static std::unordered_map<std::string, size_t> file_path_usage_count{};

bool Sprite::use_cache(Sprite &sprite, const std::string &file_path)
{
  if (sprite_cache.contains(file_path))
  {
    sprite = sprite_cache[file_path];
    file_path_usage_count[file_path] += 1;

    return true;
  }

  return false;
}

Sprite::Sprite(const std::string &file_path, std::string tag_name)
  : path{ file_path }
{
  if (use_cache(*this, file_path))
    return;

  if (path.ends_with(".aseprite"))
  {
    load_texture_with_animation();
    set_tag(tag_name);
  }
  else
  {
    texture = TextureResource(LoadTexture(std::string(path).c_str()));
  }

  TraceLog(LOG_TRACE, "Sprite(%s) loaded", path.data());
  sprite_cache[path] = *this;
  file_path_usage_count[path] += 1;

  assert(IsTextureReady(texture.get()));
}

Sprite::~Sprite()
{
  file_path_usage_count[path] -= 1;
  if (file_path_usage_count[path] == 0)
  {
    TraceLog(LOG_TRACE, "Sprite(%s) unloaded", path.data());
    sprite_cache.erase(path);
  }
}

void Sprite::load_texture_with_animation()
{
  ase_t *const ase = cute_aseprite_load_from_file(path.data(), nullptr);
  if (!ase || ase->w <= 0 || ase->h <= 0)
  {
    TraceLog(LOG_ERROR, "Cannot load \"ase\" file \"%s\"", path.data());
    return;
  }

  Image image = GenImageColor(ase->w * ase->frame_count, ase->h, BLANK);

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

  frame_width  = ase->w;
  frame_height = ase->h;
  frame_count  = ase->frame_count;
  for (int i = 0; i < ase->frame_count; ++i)
  {
    const ase_frame_t *frame{ ase->frames + i };
    frame_durations.push_back(frame->duration_milliseconds);
  }

  texture = TextureResource(LoadTextureFromImage(image));
  UnloadImage(image);

  assert(ase->frame_count < std::numeric_limits<int8_t>::max() - 1);
  if (ase->tag_count > 0 && ase->frame_count > 0)
  {
    TraceLog(LOG_INFO, "Sprite(%s, %d frames) has %d tags:", path.data(), frame_count, ase->tag_count);
    for (int i = 0; i < ase->tag_count; ++i)
    {
      const auto &atag = ase->tags[i];
      TraceLog(
        LOG_INFO, "    > AnimationTag (%d) \"%s\", frames: %d - %d", i, atag.name, atag.from_frame, atag.to_frame);

      assert(atag.from_frame < std::numeric_limits<uint8_t>::max());
      assert(atag.to_frame < std::numeric_limits<uint8_t>::max());

      tags.insert(std::make_pair(
        atag.name, AnimationTag{ static_cast<uint8_t>(atag.from_frame), static_cast<uint8_t>(atag.to_frame) }));
    }
  }

  cute_aseprite_free(ase);
}

Texture2D &Sprite::get_texture()
{
  return texture.get();
}

size_t Sprite::get_width() const
{
  if (frame_width <= 0)
    return texture->width;

  return frame_width;
}

size_t Sprite::get_height() const
{
  if (frame_height <= 0)
    return texture->height;

  return frame_height;
}

void Sprite::set_centered()
{
  origin =
    Vector2{ static_cast<float>(scale.x * get_width()) / 2.0f, static_cast<float>(scale.y * get_height()) / 2.0f };
}

Rectangle Sprite::get_source_rect() const
{
  const float sprite_w{ static_cast<float>(get_width()) };
  const float sprite_h{ static_cast<float>(get_height()) };
  const float h_flip{ scale.x > 0.0f ? 1.0f : -1.0f };
  const float v_flip{ scale.y > 0.0f ? 1.0f : -1.0f };
  return Rectangle{ static_cast<float>(frame_index) * sprite_w, 0.0f, h_flip * sprite_w, v_flip * sprite_h };
}

Rectangle Sprite::get_destination_rect() const
{
  const float sprite_w{ static_cast<float>(get_width()) };
  const float sprite_h{ static_cast<float>(get_height()) };
  return Rectangle{ std::roundf(position.x + offset.x),
                    std::roundf(position.y + offset.y),
                    sprite_w * fabsf(scale.x),
                    sprite_h * fabsf(scale.y) };
}

void Sprite::draw() const noexcept
{
  DrawTexturePro(texture.get(), get_source_rect(), get_destination_rect(), origin, rotation, tint);
}

void Sprite::reset_animation()
{
  frame_index = tag.start_frame;
  frame_timer = 0;
}

void Sprite::set_frame(int frame)
{
  if (frame >= get_frame_count())
    frame = get_frame_count() - 1;
  if (frame < 0)
    frame = 0;

  frame_index = frame;
  frame_timer = 0;
}

int Sprite::get_frame() const
{
  return frame_index;
}

int Sprite::get_frame_count() const
{
  return frame_count;
}

int64_t current_time_ms()
{
  using namespace std::chrono;
  return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

bool Sprite::should_advance_frame()
{
  const int64_t time_since_epoch_ms = current_time_ms();
  const int64_t time_difference_ms  = last_time_ms != time_since_epoch_ms ? (time_since_epoch_ms - last_time_ms) : 0;
  last_time_ms                      = time_since_epoch_ms;

  if (tag.end_frame == tag.start_frame)
  {
    frame_index = tag.start_frame;
    frame_timer = 0;
    return false;
  }

  if (frame_count <= 1) [[unlikely]]
    return false;

  const int64_t frame_array_index = frame_index;
  assert(frame_array_index < frame_count);
  assert(frame_array_index >= 0);

  if (frame_index < 0)
  {
    frame_index = frame_count > 0 ? frame_count - 1 : 0;
    return false;
  }
  if (frame_index >= frame_count)
  {
    frame_index = 0;
    return false;
  }
  const int frame_duration = frame_durations[frame_array_index];
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
  if (frame_count <= 1)
    return;

  if (should_advance_frame())
  {
    frame_index += step;
    if (frame_index > tag.end_frame || frame_index >= frame_count)
      frame_index = tag.start_frame;

    if (frame_index < tag.start_frame || frame_index < 0)
      frame_index = tag.end_frame;
  }
}

void Sprite::set_tag(std::string tag_name)
{
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
