#include "resource.hpp"

#include <raylib.h>

#include <string>
#include <unordered_map>

static std::unordered_map<int, size_t> texture_counter{};

bool TextureResource::use_counter(Texture &texture)
{
  if (texture_counter.contains(texture.id))
  {
    texture_counter[texture.id] += 1;
    return true;
  }

  TraceLog(LOG_TRACE, "TextureResource(%i) loaded", texture.id);
  texture_counter[texture.id] = 1;
  return false;
}

TextureResource::TextureResource(const Texture2D &texture)
  : texture{ texture }
{
  if (use_counter(this->texture))
    return;
}

TextureResource::TextureResource(const TextureResource &other)
  : texture{ other.texture }
{
  if (use_counter(texture))
    return;
}

TextureResource::TextureResource(TextureResource &&other)
  : texture{ std::move(other.texture) }
{
  other.texture.id = 0;
}

TextureResource &TextureResource::operator=(const TextureResource &other)
{
  texture = other.texture;

  use_counter(texture);

  return *this;
}

TextureResource &TextureResource::operator=(TextureResource &&other)
{
  texture          = std::move(other.texture);
  other.texture.id = 0;
  return *this;
}

Texture &TextureResource::operator*()
{
  return texture;
}

const Texture &TextureResource::operator*() const
{
  return texture;
}

Texture *TextureResource::operator->()
{
  return &texture;
}

const Texture *TextureResource::operator->() const
{
  return &texture;
}

TextureResource::~TextureResource()
{
  texture_counter[texture.id] -= 1;

  if (texture_counter[texture.id] > 0)
    return;

  if (IsTextureReady(texture))
  {
    texture_counter.erase(texture.id);
    UnloadTexture(texture);
    TraceLog(LOG_TRACE, "TextureResource(%i) unloaded", texture.id);
  }
}

Texture &TextureResource::get()
{
  return texture;
}

const Texture &TextureResource::get() const
{
  return texture;
}
