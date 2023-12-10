#include "bullet.hpp"

#include "game.hpp"
#include "particle.hpp"

bool Bullet::update()
{
  const Color color{ 255, 100, 255, 100 }; 

  if (life == 0)
  {
    for (int i = 0; i < 5; i++)
    {
      const Vector2 velocity{ GetRandomValue(-100, 100) / 100.0f, GetRandomValue(-100, 100) / 100.0f };
      Game::get().particles->push(Particle::create(position, velocity, color));
    }
    return false;
  }

  position.x += velocity.x;
  position.y += velocity.y;

  wrap_position(position);

  life--;

  if (life % 3 == 0)
  {
    Game::get().particles->push(Particle::create(position, Vector2{ 0.0f, 0.0f }, color));
  }

  return true;
}

void Bullet::draw() const noexcept
{
  draw_wrapped(Rectangle{ position.x, position.y, 2.0f, 2.0f },
               [&](const Vector2 &P) { DrawCircle(P.x, P.y, 2.0f, PINK); });
}
