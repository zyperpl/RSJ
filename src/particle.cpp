#include "particle.hpp"

#include <cassert>
#include <cmath>

#include "asteroid.hpp"
#include "game.hpp"
#include "player.hpp"
#include "utils.hpp"

Particle Particle::create(const Vector2 &position, const Vector2 &velocity, const Color &color) noexcept
{
  Particle particle;
  particle.position = position;
  particle.velocity = velocity;
  particle.color    = color;
  return particle;
}

static const constexpr float asteroid_size_threshold[]{ 100.0f, 400.0f, 1600.0f, 6400.0f };

bool Particle::update() noexcept
{
  position.x += velocity.x;
  position.y += velocity.y;
  velocity.x *= 0.99f;
  velocity.y *= 0.99f;

  wrap_position(position);

  if (GAME.particles && GAME.particles->capacity > 0 && GAME.frame % 3 == 0)
  {
    const auto *first                 = &GAME.particles->objects[0];
    [[maybe_unused]] const auto *last = &GAME.particles->objects[GAME.particles->capacity];
    assert(this >= first && this < last);

    const size_t object_id = this - first;
    if ((GAME.frame % 2 == 0 && object_id % 2 == 0) || (GAME.frame % 2 == 1 && object_id % 2 == 1))
    {
      GAME.asteroids->for_each(
        [&](Asteroid &asteroid) -> bool
        {
          const Vector2 &asteroid_position = asteroid.position;
          const float x_diff               = position.x - asteroid_position.x;
          const float y_diff               = position.y - asteroid_position.y;
          const float distance_sqr         = x_diff * x_diff + y_diff * y_diff;
          if (distance_sqr < 25.0f)
            return true;

          if (distance_sqr < asteroid_size_threshold[asteroid.size()])
          {
            const float factor = 0.1f / sqrt(distance_sqr);
            velocity.x += x_diff * factor;
            velocity.y += y_diff * factor;
          }

          return true;
        });
    }
  }

  const auto &player             = GAME.player;
  const Vector2 &player_position = player->position;
  const float distance           = Vector2Distance(position, player_position);

  if (distance > 1.0f)
  {
    if (distance < 30.0f)
    {
      const float x_diff = position.x - player_position.x;
      const float y_diff = position.y - player_position.y;

      velocity.x -= (x_diff * 0.01f) / distance;
      velocity.y -= (y_diff * 0.01f) / distance;
    }
    if (distance < 100.0f)
    {
      velocity.x += player->velocity.x * 0.1f / distance;
      velocity.y += player->velocity.y * 0.1f / distance;
    }
  }

  if (color.a < 255)
    color.a -= 1;

  if (color.a <= 0)
    return false;

  return true;
}

void Particle::draw() const noexcept
{
  Color c = color;
  c.a     = static_cast<unsigned char>(static_cast<float>(c.a) / 255.0f * 12.0f) * 255 / 12;
  DrawPixelV(position, c);
}
