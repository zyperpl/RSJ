#pragma once

#include <string>
#include <unordered_map>

#include <raylib.h>

struct ase_t;

class Sprite
{
public:
  struct AnimationTag
  {
    int start_frame{ 0 };
    int end_frame{ 1 };

    auto operator<=>(const AnimationTag &) const = default;
  };
  typedef std::unordered_map<std::string, const AnimationTag> AnimationTags;

  [[nodiscard]] Sprite(const std::string &file_path, std::string tag = {});
  Sprite(const Sprite &) = delete;
  Sprite(Sprite &&);
  Sprite &operator=(const Sprite &) = delete;
  Sprite &operator=(Sprite &&);

  virtual ~Sprite();
  virtual void draw() const noexcept;

  [[nodiscard]] Texture2D &get_texture();

  virtual size_t get_width() const;
  virtual size_t get_height() const;

  Rectangle get_rect() const;

  void set_frame(int frame);
  int get_frame() const;

  int get_frame_count() const;
  void set_tag(std::string tag_name);
  void reset_animation();
  void animate(int step = 1);

  void set_centered();

  Vector2 position{ 0.0f, 0.0f };
  Vector2 origin{ 0.0f, 0.0f };
  Vector2 offset{ 0.0f, 0.0f };
  Vector2 scale{ 1.0f, 1.0f };

  float rotation{ 0.0f };
  Color tint{ WHITE };

protected:
  [[nodiscard]] bool should_advance_frame();

  ase_t *ase{ nullptr };
  Texture2D texture{};
  std::string path{};

  friend bool use_cache(Sprite &, const std::string &);

private:
  void load_texture_with_animation();

  AnimationTags tags;
  AnimationTag default_tag;

  AnimationTag tag{ 0, 1 };
  int frame_index{ 0 };
  int64_t frame_timer{ 0 };

  int64_t last_time_ms{ 0 };
};
