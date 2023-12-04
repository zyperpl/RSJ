#include "game_object.hpp"

#include "game.hpp"

bool GameObject::is_colliding(const GameObject *const obj, float offset_x, float offset_y) const
{
  if (obj == this) [[unlikely]]
    return false;

  if (!obj->collidable)
    return false;

  ::Rectangle rect       = { position.x + offset_x, position.y + offset_y, mask_size.x, mask_size.y };
  ::Rectangle other_rect = { obj->position.x, obj->position.y, obj->mask_size.x, obj->mask_size.y };

  if (CheckCollisionRecs(rect, other_rect))
  {
    if (obj->one_way) [[unlikely]]
    {
      if (position.y + mask_size.y > obj->position.y)
        return false;
    }
    return true;
  }

  return false;
}

std::vector<GameObject *> GameObject::colliding(float offset_x, float offset_y) const
{
  std::vector<GameObject *> colliders;

  for (const auto &pobj : Game::objects)
  {
    GameObject *obj = pobj.get();

    if (is_colliding(obj, offset_x, offset_y))
      colliders.push_back(obj);
  }

  return colliders;
}

void GameObject::move_on_axis(float &tposition,
                              const float &tvelocity,
                              float &taccumulated_velocity,
                              float x_offset,
                              float y_offset)
{
  int amount         = static_cast<int>((std::roundf(tvelocity + taccumulated_velocity)));
  const int step_dir = std::signbit(tvelocity) ? -1 : 1;

  x_offset *= static_cast<float>(step_dir);
  y_offset *= static_cast<float>(step_dir);

  taccumulated_velocity += tvelocity - static_cast<float>(amount);

  while (amount != 0)
  {
    auto move_colliders = colliding(x_offset, y_offset);
    if (move_colliders.empty()) [[likely]]
      tposition += (x_offset + y_offset);
    else
    {
      break;
    }
    amount -= step_dir;
  }
}
