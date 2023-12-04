#pragma once

#include <cassert>
#include <cstdlib>
#include <string_view>
#include <unordered_map>

#include "sprite.hpp"

class AnimationSprite : public Sprite
{
public:
  AnimationSprite(std::string_view, std::string_view tag_name = "");

  void draw() const noexcept override;

  void set_tag(std::string_view tag_name);
  void reset_animation() { frame_index = tag.start_frame; }
  void animate(int step = 1);
  bool should_advance_frame();

  int get_frame() const { return frame_index; }
  void set_frame(int index) { frame_index = index; }
  int get_frame_count() const;
  void set_random_frame();

private:
  struct Tag
  {
    int start_frame{ 0 };
    int end_frame{ 1 };

    auto operator<=>(const Tag &) const = default;
  };
  std::unordered_map<std::string_view, const Tag> tags;
  const Tag default_tag;

  Tag tag{ 0, 1 };
  int frame_index{ 0 };
  int64_t frame_timer{ 0 };

  int64_t last_time_since_epoch_ms{ 0 };
};
