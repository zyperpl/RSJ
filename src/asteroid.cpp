#include "asteroid.hpp"

#include <cassert>

#include "bullet.hpp"
#include "game.hpp"
#include "utils.hpp"

static constexpr const float ASTEROIDS_SIZE[]   = { 8.0f, 16.0f, 32.0f };
static constexpr const int ASTEROID_SPLIT_COUNT = 2;

[[nodiscard]] Asteroid Asteroid::create(const Vector2 &position, int size)
{
  assert(size >= 0 && size < 3);

  const float speed_factor = 0.5f + (4.0f - static_cast<float>(size)) * 0.25f * 0.5f;
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

ObjectState Asteroid::update()
{
  position.x += velocity.x;
  position.y += velocity.y;

  wrap_position(position);

  for (size_t i = Game::get().bullets->tail; i < Game::get().bullets->head; i++)
  {
    Bullet &bullet = Game::get().bullets->objects[i];
    if (bullet.life <= 0)
      continue;

    const Mask bullet_mask{ bullet.position, { Circle{ Vector2{ 0.0f, 0.0f }, 2.0f } } };

    if (mask.check_collision(bullet_mask))
    {
      die();
      bullet.life = 0;
      return ObjectState::DEAD;
    }
  }

  mask.position = position;

  return ObjectState::ALIVE;
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
}

void Asteroid::draw() const noexcept
{
  sprite.position = position;
  draw_wrapped(sprite.get_rect(),
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
                 if (Game::CONFIG.show_debug)
                 {
                   DrawLineEx(P, Vector2{ P.x + velocity.x * 30.0f, P.y + velocity.y * 30.0f }, 1.0f, RED);
                   DrawPixelV(P, PINK);
                   DrawText(TextFormat("%d", size), P.x, P.y, 10, RED);
                 }
               });
}
