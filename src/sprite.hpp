#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include <raylib.h>

struct ase_t;

class Sprite
{
public:
  Sprite(std::string_view);
  virtual ~Sprite();

  virtual void draw() const noexcept;

  Texture2D &get_texture() { return texture; }

  virtual size_t get_width() const;
  virtual size_t get_height() const;

  Vector2 position{ 0.0f, 0.0f };
  Vector2 offset{ 0.0f, 0.0f };
  Vector2 scale{ 1.0f, 1.0f };

  float rotation{ 0.0f };
  Color tint{ WHITE };

  Texture2D texture;

  std::string path;

protected:
  ase_t *ase{ nullptr };
};

class SpriteSlice : public Sprite
{
public:
  SpriteSlice(std::string_view, ::Rectangle cut_mask);

  void draw() const noexcept override;

  size_t get_width() const override { return mask.width; }
  size_t get_height() const override { return mask.height; }

  void set_mask(::Rectangle new_mask) { mask = new_mask; }
  ::Rectangle get_mask() const { return mask; }

private:
  ::Rectangle mask{ 0.0f, 0.0f, 0.0f, 0.f };
};
