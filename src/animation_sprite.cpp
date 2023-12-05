#include "animation_sprite.hpp"

#include <chrono>

#include <cmath>

#include <cute_aseprite.h>
#include <raylib.h>

AnimationSprite::AnimationSprite(std::string_view file_path, std::string_view tag_name)
  : Sprite{ file_path }
  , default_tag{ 0, ase->frame_count - 1 }
  , tag{ default_tag }
{
  for (int i = 0; i < ase->tag_count; ++i)
  {
    const auto &atag = ase->tags[i];
    printf(
      "Tag \"%s\", from %d to %d (out of %d frames).\n", atag.name, atag.from_frame, atag.to_frame, ase->frame_count);
    tags.insert(std::make_pair(atag.name, Tag{ atag.from_frame, atag.to_frame }));
  }

  set_tag(tag_name);
}

int AnimationSprite::get_frame_count() const
{
  if (!ase) [[unlikely]]
    return 0;
  return ase->frame_count;
}

bool AnimationSprite::should_advance_frame()
{
  using namespace std::chrono;
  const int64_t time_since_epoch_ms =
    duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();

  const int64_t time_difference_ms =
    last_time_since_epoch_ms != time_since_epoch_ms ? (time_since_epoch_ms - last_time_since_epoch_ms) : 0;
  last_time_since_epoch_ms = time_since_epoch_ms;

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

void AnimationSprite::animate(int step)
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

void AnimationSprite::set_random_frame()
{
  const auto min_frame         = tag.start_frame;
  const auto max_frame         = tag.end_frame;
  const int random_frame_index = GetRandomValue(min_frame, max_frame);

  frame_index = random_frame_index;

  assert(frame_index >= 0);
  assert(frame_index <= ase->frame_count);
  assert(frame_index >= tag.start_frame);
  assert(frame_index <= tag.end_frame);
}

void AnimationSprite::set_tag(std::string_view tag_name)
{
  if (!ase) [[unlikely]]
    return;

  if (tag_name != "")
  {
    assert(tags.contains(tag_name));
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

void AnimationSprite::draw() const noexcept
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

  const Vector2 origin = { 0, 0 };
  DrawTexturePro(texture, source, dest, origin, rotation, tint);
}
