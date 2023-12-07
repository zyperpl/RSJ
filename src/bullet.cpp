#include "bullet.hpp"

ObjectState Bullet::update()
{
  if (life == 0)
    return ObjectState::DEAD;

  position.x += velocity.x;
  position.y += velocity.y;

  //wrap_position(position);

  life--;

  return ObjectState::ALIVE;
}

void Bullet::draw() const noexcept
{
  DrawCircle(position.x, position.y, 2.0f, PINK);

  const float s = 0.1f;
  DrawSphereEx(Vector3{ position.x * s, 0.0f, position.y * s }, 0.2f, 4, 4, WHITE);
}
