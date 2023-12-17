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
                  GAME.coins += 1;
                  GAME.score += 100;
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

  if (!GAME.player)
    return true;

  if (!player)
  {
    if (mask.check_collision(GAME.player->get_mask()))
    {
      player = GAME.player.get();

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

    if (d > std::min(Game::width, Game::height) / 2.0f)
    {
      position.x = player->position.x - dir.x * 7.0f;
      position.y = player->position.y - dir.y * 7.0f;
    }

    velocity.x += dir.x * 0.1f;
    velocity.y += dir.y * 0.1f;

    position.x += dir.x * 0.4f;
    position.y += dir.y * 0.4f;
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
