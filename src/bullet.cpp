#include "bullet.hpp"

bool Bullet::update()
{
  if (life == 0)
    return false;

  position.x += velocity.x;
  position.y += velocity.y;

  wrap_position(position);

  life--;

  return true;
}

void Bullet::draw() const noexcept
{
  draw_wrapped(Rectangle{ position.x, position.y, 2.0f, 2.0f },
               [&](const Vector2 &P) { DrawCircle(P.x, P.y, 2.0f, PINK); });
}
