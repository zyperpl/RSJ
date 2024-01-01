#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <raylib.h>
#include <raymath.h>

class TextureResource
{
public:
  TextureResource() = default;
  [[nodiscard]] explicit TextureResource(const Texture2D &texture);
  [[nodiscard]] TextureResource(const TextureResource &);
  [[nodiscard]] TextureResource(TextureResource &&);

  TextureResource &operator=(const TextureResource &);
  TextureResource &operator=(TextureResource &&);

  [[nodiscard]] Texture &operator*();
  [[nodiscard]] const Texture &operator*() const;
  [[nodiscard]] Texture *operator->();
  [[nodiscard]] const Texture *operator->() const;

  ~TextureResource();

  [[nodiscard]] Texture &get();
  [[nodiscard]] const Texture &get() const;

private:
  Texture texture{};

  static bool use_counter(Texture &texture);
};
