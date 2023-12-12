#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <raylib.h>
#include <raymath.h>

#include "resource.hpp"

class Sprite final
{
public:
  struct AnimationTag
  {
    int start_frame{ 0 };
    int end_frame{ 1 };

    auto operator<=>(const AnimationTag &) const = default;
  };
  typedef std::unordered_map<std::string, AnimationTag> AnimationTags;

  Sprite() = default;
  [[nodiscard]] Sprite(const std::string &file_path, std::string tag = {});

  ~Sprite();
  void draw() const noexcept;

  [[nodiscard]] Texture2D &get_texture();

  [[nodiscard]] size_t get_width() const;
  [[nodiscard]] size_t get_height() const;

  [[nodiscard]] Rectangle get_source_rect() const;
  [[nodiscard]] Rectangle get_destination_rect() const;

  void set_frame(int frame);
  [[nodiscard]] int get_frame() const;

  [[nodiscard]] int get_frame_count() const;
  void set_tag(std::string tag_name);
  void reset_animation();
  void animate(int step = 1);

  void set_centered();

  Vector2 position{ 0.0f, 0.0f };
  Vector2 origin{ 0.0f, 0.0f };
  Vector2 offset{ 0.0f, 0.0f };
  Vector2 scale{ 1.0f, 1.0f };

  Color tint{ WHITE };
  float rotation{ 0.0f };

protected:
  [[nodiscard]] bool should_advance_frame();

  TextureResource texture{};
  std::string path{};

private:
  void load_texture_with_animation();

  AnimationTags tags;
  AnimationTag default_tag;

  AnimationTag tag{ 0, 1 };
  int frame_index{ 0 };
  size_t frame_width{ 0 };
  size_t frame_height{ 0 };
  int frame_count{ 0 };
  std::vector<int> frame_durations; // in milliseconds
  int64_t frame_timer{ 0 };

  int64_t last_time_ms{ 0 };

  static bool use_cache(Sprite &, const std::string &);
};
