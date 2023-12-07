#include "asteroid.hpp"

#include "bullet.hpp"
#include "game.hpp"
#include "utils.hpp"

static constexpr const float ASTEROIDS_MIN_SIZE = 5.0f;

[[nodiscard]] Asteroid Asteroid::create(const Vector2 &position, float rotation, float size)
{
  const float random_angle = GetRandomValue(0, 360) * DEG2RAD;
  Asteroid asteroid;
  asteroid.position       = position;
  asteroid.velocity.x     = cos(random_angle + rotation * DEG2RAD + M_PI / 2.0f + M_PI) * 1.0f;
  asteroid.velocity.y     = sin(random_angle + rotation * DEG2RAD + M_PI / 2.0f + M_PI) * 1.0f;
  asteroid.rotation_speed = GetRandomValue(-1, 1) * 0.01f;
  asteroid.size           = size;
  return asteroid;
}

ObjectState Asteroid::update()
{
  position.x += velocity.x;
  position.y += velocity.y;

  wrap_position(position);

  rotation += rotation_speed;

  for (size_t i = Game::get().bullets->tail; i < Game::get().bullets->head; i++)
  {
    Bullet &bullet = Game::get().bullets->objects[i];
    if (bullet.life <= 0)
      continue;

    if (CheckCollisionCircles(position, size, bullet.position, 2.0f))
    {
      bullet.life = 0;

      if (size > ASTEROIDS_MIN_SIZE)
      {
        Game::get().asteroids->push(Asteroid::create(position, rotation, size * 0.5f));
        Game::get().asteroids->push(Asteroid::create(position, rotation, size * 0.5f));
      }

      return ObjectState::DEAD;
    }
  }

  return ObjectState::ALIVE;
}

void Asteroid::draw() const noexcept
{
  DrawCircleLines(position.x, position.y, size, RED);
  DrawCircleLines(position.x + Game::width, position.y, size, RED);
  DrawCircleLines(position.x - Game::width, position.y, size, RED);
  DrawCircleLines(position.x, position.y + Game::height, size, RED);
  DrawCircleLines(position.x, position.y - Game::height, size, RED);
}
