#pragma once

#include <raylib.h>

class Game;

enum class RenderPass
{
  Normal,
  Shadow,
  Bloom
};

class Renderer
{
public:
  Renderer();
  ~Renderer();

  void render_game_objects(Game &game) const;
  void render_menu(Game &game) const;
  void render_game() const;
  void render_screen(const Rectangle &render_destination) const;

private:
  RenderTexture2D game_render_target;
  RenderTexture2D menu_render_target;
  RenderTexture2D objects_render_target;

  Rectangle render_source;
};
