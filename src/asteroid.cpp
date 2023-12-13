#include "asteroid.hpp"

#include <cassert>

#include "bullet.hpp"
#include "game.hpp"
#include "particle.hpp"
#include "utils.hpp"

static constexpr const float ASTEROIDS_SIZE[]   = { 8.0f, 16.0f, 32.0f };
static constexpr const int ASTEROID_SPLIT_COUNT = 2;

[[nodiscard]] Asteroid Asteroid::create(const Vector2 &position, int size)
{
  assert(size >= 0 && size < 3);

  const float speed_factor = 0.5f + (4.0f - static_cast<float>(size)) * 0.3f * 0.5f;
  const float random_angle = (static_cast<float>(GetRandomValue(0, 100)) / 100.0f) * M_PI * 2.0f;
  Asteroid asteroid;
  asteroid.position     = position;
  asteroid.velocity.x   = cos(random_angle) * speed_factor;
  asteroid.velocity.y   = sin(random_angle) * speed_factor;
  asteroid.size         = size;
  const float mask_size = ASTEROIDS_SIZE[size] * 0.5f;
  asteroid.mask.shapes.push_back(Circle{ Vector2{ 0.0f, 0.0f }, mask_size });
  asteroid.sprite.set_frame(3 - size - 1);
  asteroid.sprite.set_centered();
  return asteroid;
}

bool Asteroid::update()
{
  position.x += velocity.x;
  position.y += velocity.y;

  wrap_position(position);

  Game::get().bullets->for_each(
    [&](Bullet &bullet) -> bool
    {
      if (bullet.life <= 0)
        return true;

      const Mask bullet_mask{ bullet.position, { Circle{ Vector2{ 0.0f, 0.0f }, 2.0f } } };

      if (mask.check_collision(bullet_mask))
      {
        die();
        bullet.life = 0;
        life--;
        return false;
      }

      return true;
    });

  mask.position = position;

  if (life <= 0)
    return false;

  return true;
}

void Asteroid::die()
{
  if (size > 0)
  {
    for (size_t i = 0; i < ASTEROID_SPLIT_COUNT; i++)
    {
      Game::get().asteroids->push(Asteroid::create(position, size - 1));
    }
  }

  for (int i = 0; i < 20 - std::max(1, size * 5); i++)
  {
    const Vector2 pos{ position.x + static_cast<float>(GetRandomValue(-10, 10)),
                       position.y + static_cast<float>(GetRandomValue(-10, 10)) };
    const Vector2 vel = Vector2Normalize(
      Vector2{ static_cast<float>(GetRandomValue(-100, 100)), static_cast<float>(GetRandomValue(-100, 100)) });
    const float hue        = 229.0f - 10.0f + static_cast<float>(GetRandomValue(0, 20));
    const float saturation = 0.3f + static_cast<float>(GetRandomValue(0, 10)) / 100.0f;
    const float value      = 0.1f + static_cast<float>(GetRandomValue(0, 50)) / 100.0f;
    const Color color      = ColorFromHSV(hue, saturation, value);
    Game::get().particles->push(Particle::create(pos, vel, color));
  }
}

void Asteroid::draw() const noexcept
{
  sprite.position = position;
  draw_wrapped(sprite.get_destination_rect(),
               [&](const Vector2 &P)
               {
                 sprite.position = P;
                 sprite.draw();

                 if (Game::CONFIG.show_masks)
                 {
                   Mask mask_copy     = mask;
                   mask_copy.position = P;
                   mask_copy.draw();
                 }
               });

  if (Game::CONFIG.show_debug)
  {
    DrawPixelV(position, PINK);
    DrawText(TextFormat("%d", size), position.x, position.y, 10, RED);
  }
  if (Game::CONFIG.show_velocity)
    DrawLineEx(position, Vector2{ position.x + velocity.x * 20.0f, position.y + velocity.y * 20.0f }, 1.0f, RED);
}
