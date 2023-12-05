#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include <raylib.h>

struct ase_t;

class Sprite
{
public:
  [[nodiscard]] Sprite(std::string_view file_path, std::string_view tag = {});
  Sprite(const Sprite &) = delete;
  Sprite(Sprite &&) = delete;
  Sprite &operator=(const Sprite &) = delete;
  Sprite &operator=(Sprite &&) = delete;

  virtual ~Sprite();
  virtual void draw() const noexcept;

  [[nodiscard]] Texture2D &get_texture();

  virtual size_t get_width() const;
  virtual size_t get_height() const;

  void set_frame(int frame);
  int get_frame() const;

  int get_frame_count() const;
  void set_tag(std::string_view tag_name);
  void reset_animation();
  void animate(int step = 1);

  Vector2 position{ 0.0f, 0.0f };
  Vector2 offset{ 0.0f, 0.0f };
  Vector2 scale{ 1.0f, 1.0f };

  float rotation{ 0.0f };
  Color tint{ WHITE };

protected:
  [[nodiscard]] bool should_advance_frame();

  ase_t *ase{ nullptr };
  Texture2D texture{};
  std::string path{};

private:
  void load_texture_with_animation();

  struct AnimationTag
  {
    int start_frame{ 0 };
    int end_frame{ 1 };

    auto operator<=>(const AnimationTag &) const = default;
  };

  std::unordered_map<std::string_view, const AnimationTag> tags;
  AnimationTag default_tag;

  AnimationTag tag{ 0, 1 };
  int frame_index{ 0 };
  int64_t frame_timer{ 0 };

  int64_t last_time_ms{ 0 };
};
