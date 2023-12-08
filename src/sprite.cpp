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

static std::unordered_map<std::string, Texture2D> texture_cache{};
static std::unordered_map<std::string, ase_t *> aseprite_files_cache{};
static std::unordered_map<std::string, Sprite::AnimationTags> tags_cache{};
static std::unordered_map<std::string, size_t> file_path_usage_count{};

bool use_cache(Sprite &sprite, const std::string &file_path)
{
  if (auto texture_cache_iterator = texture_cache.find(file_path); texture_cache_iterator != texture_cache.end())
  {
    sprite.texture = texture_cache_iterator->second;

    if (auto aseprite_files_cache_iterator = aseprite_files_cache.find(file_path);
        aseprite_files_cache_iterator != aseprite_files_cache.end())
      sprite.ase = aseprite_files_cache_iterator->second;

    if (auto tags_cache_iterator = tags_cache.find(file_path); tags_cache_iterator != tags_cache.end())
      sprite.tags = tags_cache_iterator->second;

    file_path_usage_count[file_path] += 1;

    return true;
  }

  return false;
}

Sprite::Sprite(const std::string &file_path, std::string tag_name)
  : path{ file_path }
{
  if (use_cache(*this, file_path))
  {
    TraceLog(
      LOG_TRACE, "Sprite(%s) loaded from cache (usages: %zu)", file_path.data(), file_path_usage_count[file_path]);
    return;
  }

  if (path.ends_with(".aseprite"))
  {
    load_texture_with_animation();
    set_tag(tag_name);

    if (ase)
    {
      aseprite_files_cache[path] = ase;
      tags_cache[path]           = tags;
    }
  }
  else
  {
    texture = LoadTexture(std::string(path).c_str());
  }
  texture_cache[path] = texture;
  file_path_usage_count[path] += 1;

  assert(IsTextureReady(texture));
}

Sprite::Sprite(Sprite &&other)
  : position{ other.position }
  , origin{ other.origin }
  , offset{ other.offset }
  , scale{ other.scale }
  , rotation{ other.rotation }
  , tint{ other.tint }
  , ase{ other.ase }
  , texture{ std::move(other.texture) }
  , path{ std::move(other.path) }
  , tags{ std::move(other.tags) }
  , default_tag{ other.default_tag }
  , tag{ other.tag }
  , frame_index{ other.frame_index }
  , frame_timer{ other.frame_timer }
  , last_time_ms{ other.last_time_ms }
{
  other.texture = {};
  other.ase     = nullptr;
}

Sprite &Sprite::operator=(Sprite &&other)
{
  position     = other.position;
  origin       = other.origin;
  offset       = other.offset;
  scale        = other.scale;
  rotation     = other.rotation;
  tint         = other.tint;
  ase          = other.ase;
  texture      = std::move(other.texture);
  path         = std::move(other.path);
  tags         = std::move(other.tags);
  default_tag  = other.default_tag;
  tag          = other.tag;
  frame_index  = other.frame_index;
  frame_timer  = other.frame_timer;
  last_time_ms = other.last_time_ms;

  other.texture = {};
  other.ase     = nullptr;

  return *this;
}

Sprite::~Sprite()
{
  file_path_usage_count[path] -= 1;
  if (file_path_usage_count[path] > 0)
    return;

  UnloadTexture(texture);

  if (ase)
    cute_aseprite_free(ase);
}

void Sprite::load_texture_with_animation()
{
  ase = cute_aseprite_load_from_file(path.data(), nullptr);
  if (!ase || ase->frame_count <= 0 || ase->w <= 0 || ase->h <= 0)
  {
    TraceLog(LOG_ERROR, "Cannot load ase file \"%s\"", path.data());
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

  texture = LoadTextureFromImage(image);
  UnloadImage(image);

  if (ase->tag_count > 0 && ase->frame_count > 0)
  {
    TraceLog(LOG_INFO, "Sprite(%s, %d frames) has %d tags:", path.data(), ase->frame_count, ase->tag_count);

    for (int i = 0; i < ase->tag_count; ++i)
    {
      const auto &atag = ase->tags[i];
      TraceLog(LOG_INFO, "\t%d. AnimationTag \"%s\", frames: %d - %d", i, atag.name, atag.from_frame, atag.to_frame);
      tags.insert(std::make_pair(atag.name, AnimationTag{ atag.from_frame, atag.to_frame }));
    }
  }
  else
  {
    if (ase->frame_count <= 0)
      TraceLog(LOG_WARNING, "Sprite(%s) has no frames", path.data());
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

void Sprite::set_centered()
{
  origin = Vector2{ static_cast<float>(get_width()) / 2.0f, static_cast<float>(get_height()) / 2.0f };
}

Rectangle Sprite::get_rect() const
{
  return Rectangle{ position.x, position.y, static_cast<float>(get_width()), static_cast<float>(get_height()) };
}

void Sprite::draw() const noexcept
{
  const float h_flip{ scale.x > 0.0f ? 1.0f : -1.0f };
  const float v_flip{ scale.y > 0.0f ? 1.0f : -1.0f };

  const float sprite_w{ static_cast<float>(get_width()) };
  const float sprite_h{ static_cast<float>(get_height()) };

  const Rectangle source{ (float)(frame_index)*sprite_w, 0.0f, h_flip * sprite_w, v_flip * sprite_h };
  const Rectangle dest{ std::roundf(position.x + offset.x),
                        std::roundf(position.y + offset.y),
                        sprite_w * fabsf(scale.x),
                        sprite_h * fabsf(scale.y) };

  DrawTexturePro(texture, source, dest, origin, rotation, tint);
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
  if (!ase) [[unlikely]]
    return 0;
  return ase->frame_count;
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

void Sprite::set_tag(std::string tag_name)
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
