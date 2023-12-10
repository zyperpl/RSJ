#include "particle.hpp"

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

bool Particle::update() noexcept
{
  position.x += velocity.x;
  position.y += velocity.y;
  velocity.x *= 0.99f;
  velocity.y *= 0.99f;

  wrap_position(position);

  Game::get().asteroids->for_each(
    [&](Asteroid &asteroid)
    {
      const Vector2 &asteroid_position = asteroid.position;

      const float distance = Vector2Distance(position, asteroid_position);
      if (distance < 5.0f)
        return true;

      const float x_diff = position.x - asteroid_position.x;
      const float y_diff = position.y - asteroid_position.y;

      if (distance < asteroid.size * 10.0f)
      {
        velocity.x += (x_diff * 0.1f) / distance;
        velocity.y += (y_diff * 0.1f) / distance;
      }

      return true;
    });

  const auto &player             = Game::get().player;
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

  const float particle_usage =
    static_cast<float>(Game::get().particles->size()) / static_cast<float>(Game::get().particles->capacity);
  if ((particle_usage > 0.9f && color.a >= 255 && Vector2Length(velocity) < 0.1f) || color.a < 255)
    color.a -= 1;

  if (color.a <= 0)
    return false;

  return true;
}

void Particle::draw() const noexcept
{
  DrawPixelV(position, color);
}
