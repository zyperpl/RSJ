#include "bullet.hpp"

#include "asteroid.hpp"
#include "game.hpp"
#include "particle.hpp"
#include "utils.hpp"

Bullet Bullet::create_normal(const Vector2 &position, const Vector2 &velocity)
{
  Bullet bullet;
  bullet.position  = position;
  bullet.velocity  = velocity;
  bullet.direction = Vector2Normalize(velocity);
  bullet.type      = BulletType::Normal;
  bullet.life      = 30;
  return bullet;
}

#if defined(DEBUG)
static Vector2 DEBUG_asteroid_position;
#endif

const Asteroid &get_nearest_asteroid(const Vector2 &position)
{
  auto &asteroids                  = GAME.asteroids;
  const Asteroid *nearest_asteroid = &asteroids->objects[0];
  float nearest_distance{ std::numeric_limits<float>::max() };

  asteroids->for_each(
    [&](const Asteroid &asteroid)
    {
      const float distance = Vector2Distance(position, asteroid.position);
      if (distance < nearest_distance)
      {
        nearest_asteroid = &asteroid;
        nearest_distance = distance;
      }
    });

  return *nearest_asteroid;
}

Bullet Bullet::create_assisted(const Vector2 &position, const Vector2 &velocity)
{
  Bullet bullet;
  bullet.position        = position;
  bullet.velocity        = velocity;
  Vector2 check_position = Vector2Add(position, Vector2Scale(velocity, 10.0f));

#if defined(DEBUG)
  DEBUG_asteroid_position = get_nearest_asteroid(check_position).position;
#endif
  bullet.direction = Vector2Normalize(Vector2Subtract(get_nearest_asteroid(check_position).position, position));
  bullet.type      = BulletType::Assisted;
  bullet.life      = 40;
  return bullet;
}

Bullet Bullet::create_homing(const Vector2 &position, [[maybe_unused]] const Vector2 &velocity)
{
  Bullet bullet;
  bullet.position        = position;
  bullet.direction       = Vector2Normalize(velocity);
  auto &nearest_asteroid = get_nearest_asteroid(position);
  if (nearest_asteroid.life > 0)
    bullet.target = &nearest_asteroid.position;
  bullet.type = BulletType::Homing;
  bullet.life = 20;
  return bullet;
}

bool Bullet::update()
{
  const Color particle_color{ 255, 100, 255, 80 };

  if (life == 0)
  {
    const size_t number_of_particles = 5;
    for (size_t i = 0; i < number_of_particles; ++i)
    {
      const Vector2 velocity{ GetRandomValue(-100, 100) / 100.0f, GetRandomValue(-100, 100) / 100.0f };
      GAME.particles->push(Particle::create(position, velocity, particle_color));
    }
    return false;
  }

  if (type == BulletType::Assisted)
  {
    velocity.x += direction.x * 0.8f;
    velocity.y += direction.y * 0.8f;

    velocity = Vector2Scale(Vector2Normalize(velocity), 5.0f);
  }
  else if (type == BulletType::Homing && target != nullptr)
  {
    direction = Vector2Normalize(Vector2Subtract(get_target_position(), position));

    velocity.x += direction.x * 2.0f;
    velocity.y += direction.y * 2.0f;

    velocity = Vector2Scale(Vector2Normalize(velocity), 6.0f);
  }

  position.x += velocity.x;
  position.y += velocity.y;

  wrap_position(position);

  life--;

  size_t particles_per_frame = 3;
  if (type == BulletType::Homing)
    particles_per_frame = 1;
  if (life % particles_per_frame == 0)
  {
    GAME.particles->push(Particle::create(position, Vector2{ 0.0f, 0.0f }, particle_color));
  }

  return true;
}

void Bullet::draw() const noexcept
{
  Color color{ PINK };

  if (type == BulletType::Homing)
    color = ORANGE;

  draw_wrapped(Rectangle{ position.x, position.y, 2.0f, 2.0f },
               [&](const Vector2 &P) { DrawCircle(P.x, P.y, 2.0f, color); });

#if defined(DEBUG)
  if (CONFIG(debug_bullets))
  {
    DrawCircleV(DEBUG_asteroid_position, 2.0f, RED);
    DrawCircleLinesV(DEBUG_asteroid_position, 20.0f, RED);

    if (target)
    {
      DrawCircleV(get_target_position(), 2.0f, RED);
      DrawCircleLinesV(get_target_position(), 20.0f, RED);
    }
  }
#endif
}

Vector2 Bullet::get_target_position() const noexcept
{
  if (target)
    return *target;
  return position;
}
