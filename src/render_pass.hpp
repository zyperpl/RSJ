#pragma once

#include <cassert>
#include <functional>
#include <memory>

#include <raylib.h>
#include <raymath.h>

#include "utils.hpp"

class RenderPass
{
public:
  RenderTexture2D render_texture{};
  std::function<void()> render_func;

  RenderPass(int width, int height);

  ~RenderPass();

  void render();

  void draw(const Rectangle &render_destination);
};
