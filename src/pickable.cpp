#include "pickable.hpp"

#include <functional>

#include "game.hpp"
#include "particle.hpp"
#include "player.hpp"
#include "utils.hpp"

static std::unique_ptr<Sprite> ORE_SPRITE;

Pickable Pickable::create(const Vector2 &position, const Vector2 &velocity, const std::function<void()> &func)
{
  if (!ORE_SPRITE)
    ORE_SPRITE = std::make_unique<Sprite>("resources/ore.aseprite");

  Pickable pickable(position, func);
  pickable.velocity = velocity;
  return pickable;
}

void ore_func()
{
  GAME.coins += 1;
  GAME.score += 100;
}

Pickable Pickable::create_ore(const Vector2 &position, const Vector2 &velocity)
{
  return create(position, velocity, ore_func);
}

Pickable::Pickable(const Vector2 &position, const std::function<void()> &func)
  : position{ position }
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

  const auto &player = GAME.player;

  if (player_id == -1)
  {
    if (mask.check_collision(player->get_mask()))
    {
      player_id = 1;

      velocity = Vector2Normalize(Vector2Subtract(position, player->position));
      velocity.x *= 1.1f;
      velocity.y *= 1.1f;
    }
  }
  else
  {
    const float player_distance = Vector2Distance(position, player->position);
    if (player_distance < 8.0f)
    {
      if (func)
        func();

      return false;
    }
    const Vector2 dir = Vector2Normalize(Vector2Subtract(player->position, position));

    if (player_distance > std::min(Game::width, Game::height) / 2.0f)
    {
      position.x = player->position.x - dir.x * 7.0f;
      position.y = player->position.y - dir.y * 7.0f;
    }

    velocity.x += dir.x * 0.1f;
    velocity.y += dir.y * 0.1f;

    position.x += dir.x * 0.5f;
    position.y += dir.y * 0.5f;
  }

  return true;
}

void Pickable::draw() const
{
  if (player_id != -1)
    ORE_SPRITE->scale = Vector2{ 0.86f, 0.86f };
  else
    ORE_SPRITE->scale = Vector2{ 1.0f, 1.0f };

  ORE_SPRITE->set_centered();
  ORE_SPRITE->position = position;
  draw_wrapped(ORE_SPRITE->get_destination_rect(),
               [&](const Vector2 &position)
               {
                 ORE_SPRITE->position = position;
                 ORE_SPRITE->draw();
               });
}
