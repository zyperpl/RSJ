#include "bullet.hpp"

ObjectState Bullet::update()
{
  if (life == 0)
    return ObjectState::DEAD;

  position.x += velocity.x;
  position.y += velocity.y;

  wrap_position(position);

  life--;

  return ObjectState::ALIVE;
}

void Bullet::draw() const noexcept
{
  DrawCircle(position.x, position.y, 2.0f, PINK);
}
