#pragma once

#include <cmath>
#include <memory>
#include <vector>

#include <raylib.h>

#include "renderer.hpp"
#include "sprite.hpp"

enum class HorizontalAlign
{
  None,
  Left,
  Center,
  Right
};

enum class VerticalAlign
{
  None,
  Top,
  Center,
  Bottom
};

class GameObject
{
public:
  virtual ~GameObject() = default;

  virtual void draw(const RenderPass &render_pass)
  {
    if (render_pass == RenderPass::Normal)
    {
      sprite->position = position;
      sprite->draw();
    }
  }

  bool is_colliding(const GameObject *const obj, float offset_x, float offset_y) const;

  std::vector<GameObject *> colliding(float offset_x, float offset_y) const;

  inline void move_on_axis(float &tposition,
                           const float &tvelocity,
                           float &taccumulated_velocity,
                           float x_offset,
                           float y_offset);

  virtual void step(){};

  void simulate()
  {
    move_on_axis(position.x, velocity.x, accumulated_velocity.x, 1.0f, 0.0f);
    move_on_axis(position.y, velocity.y, accumulated_velocity.y, 0.0f, 1.0f);
  }

  void update()
  {
    simulate();
    step();
  }

  virtual void hurt(){};

  std::shared_ptr<Sprite> sprite;

  void align_sprite_by_mask(Sprite &tsprite, HorizontalAlign align_x, VerticalAlign align_y)
  {
    switch (align_x)
    {
      case HorizontalAlign::Left:
        tsprite.offset.x = 0.0f;
        break;

      case HorizontalAlign::Center:
        tsprite.offset.x = -(static_cast<float>(tsprite.get_width()) - mask_size.x) / 2.0f;
        break;

      case HorizontalAlign::Right:
        tsprite.offset.x = -(static_cast<float>(tsprite.get_width()) - mask_size.x);
        break;

      case HorizontalAlign::None:
        break;
    }

    switch (align_y)
    {
      case VerticalAlign::Top:
        tsprite.offset.y = 0.0f;
        break;

      case VerticalAlign::Center:
        tsprite.offset.y = -(static_cast<float>(tsprite.get_height()) - mask_size.y) / 2.0f;
        break;

      case VerticalAlign::Bottom:
        tsprite.offset.y = -(static_cast<float>(tsprite.get_height()) - mask_size.y);
        break;

      case VerticalAlign::None:
        break;
    }
  }

  void align_sprite_by_self(Sprite &tsprite, HorizontalAlign align_x, VerticalAlign align_y)
  {
    switch (align_x)
    {
      case HorizontalAlign::Left:
        tsprite.offset.x = 0.0f;
        break;

      case HorizontalAlign::Center:
        tsprite.offset.x = -static_cast<float>(tsprite.get_width()) / 2.0f;
        break;

      case HorizontalAlign::Right:
        tsprite.offset.x = -static_cast<float>(tsprite.get_width());
        break;

      case HorizontalAlign::None:
        break;
    }

    switch (align_y)
    {
      case VerticalAlign::Top:
        tsprite.offset.y = 0.0f;
        break;

      case VerticalAlign::Center:
        tsprite.offset.y = -static_cast<float>(tsprite.get_height()) / 2.0f;
        break;

      case VerticalAlign::Bottom:
        tsprite.offset.y = -static_cast<float>(tsprite.get_height());
        break;

      case VerticalAlign::None:
        break;
    }
  }

  Vector2 center() const { return Vector2{ position.x + mask_size.x / 2.0f, position.y + mask_size.y / 2.0f }; }

  Rectangle mask(float inflate = 0.0f) const
  {
    return Rectangle{
      position.x - inflate / 2.0f, position.y - inflate / 2.0f, mask_size.x + inflate, mask_size.y + inflate
    };
  }

  Vector2 position{ 0.0f, 0.0f };
  Vector2 velocity{ 0.0f, 0.0f };
  Vector2 accumulated_velocity{ 0.0f, 0.0f };
  Vector2 mask_size{ 0.0f, 0.0f };

  bool one_way{ false };
  bool collidable{ false };
};
