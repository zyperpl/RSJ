#include "pickable.hpp"

#include <functional>

#include "game.hpp"
#include "particle.hpp"
#include "player.hpp"
#include "utils.hpp"

Pickable Pickable::create(const Vector2 &position, const Vector2 &velocity, std::function<void()> func)
{
  Pickable pickable(position, func);
  pickable.velocity = velocity;
  return pickable;
}

Pickable Pickable::create_ore(const Vector2 &position, const Vector2 &velocity)
{
  return create(position,
                velocity,
                []()
                {
                  Game::get().coins += 1;
                  Game::get().score += 100;
                });
}

Pickable::Pickable(const Vector2 &position, std::function<void()> func)
  : position{ position }
  , velocity{ 0.0f, 0.0f }
  , func{ func }
{
}

bool Pickable::update()
{
  position.x += velocity.x;
  position.y += velocity.y;

  mask.position = position;
  wrap_position(position);

  if (!Game::get().player)
    return true;

  if (!player)
  {
    if (mask.check_collision(Game::get().player->mask))
    {
      player = Game::get().player.get();

      velocity = Vector2Normalize(Vector2Subtract(position, player->position));
      velocity.x *= 1.1f;
      velocity.y *= 1.1f;

      sprite.scale = Vector2{ 0.86f, 0.86f };
    }
  }
  else
  {
    const float d = Vector2Distance(position, player->position);
    if (d < 8.0f)
    {
      if (func)
        func();

      return false;
    }
    const Vector2 dir = Vector2Normalize(Vector2Subtract(player->position, position));

    if (d > 64.0f)
    {
      position.x = player->position.x - dir.x * 16.0f;
      position.y = player->position.y - dir.y * 16.0f;
    }

    velocity.x += dir.x * 0.3f;
    velocity.y += dir.y * 0.3f;
  }

  return true;
}

void Pickable::draw() const
{
  sprite.set_centered();
  sprite.position = position;
  draw_wrapped(sprite.get_destination_rect(),
               [&](const Vector2 &position)
               {
                 sprite.position = position;
                 sprite.draw();
               });
}
