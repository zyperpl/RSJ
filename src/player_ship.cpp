#include "player_ship.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#include <functional>

#include <raylib.h>
#include <raymath.h>

#include "asteroid.hpp"
#include "bullet.hpp"
#include "game.hpp"
#include "particle.hpp"
#include "utils.hpp"

PlayerShip::PlayerShip()
{
  mask.shapes.push_back(Circle{ Vector2{ 0.0f, 0.0f }, 8.0f });
  mask.position = position;

  sprite.set_centered();
  sprite.position = position;
}

void PlayerShip::draw() const noexcept
{
  const int invincibility_timer_int = static_cast<int>(invincibility_timer.get_remaining_time() * 10.0f);
  if (!invincibility_timer.is_done() && invincibility_timer_int % 2 == 0)
    return;

  sprite.set_centered();

  sprite.position = position;

  draw_wrapped(sprite.get_destination_rect(),
               [&](const Vector2 &P)
               {
                 sprite.position = P;
                 sprite.draw();

                 DrawCircleV(P, 1.0f, GREEN);

                 if (CONFIG(show_masks))
                 {
                   Mask mask_copy     = mask;
                   mask_copy.position = P;
                   mask_copy.draw();
                 }

                 if (CONFIG(show_velocity))
                   DrawLineEx(P, Vector2{ P.x + velocity.x * 10.0f, P.y + velocity.y * 10.0f }, 1.0f, RED);
               });
}

void PlayerShip::die()
{
  auto &game = Game::get();
  for (int i = 0; i < 10; ++i)
  {
    const Vector2 pos{ position.x + static_cast<float>(GetRandomValue(-20, 20)),
                       position.y + static_cast<float>(GetRandomValue(-20, 20)) };
    const Vector2 vel = Vector2Normalize(
      Vector2{ static_cast<float>(GetRandomValue(-100, 100)), static_cast<float>(GetRandomValue(-100, 100)) });
    const Color color = ColorBrightness(BLACK, 0.1f);
    game.particles->push(Particle::create(pos, vel, color));
  }
  for (int i = 0; i < 100; ++i)
  {
    const Vector2 pos{ position.x + static_cast<float>(GetRandomValue(-10, 10)),
                       position.y + static_cast<float>(GetRandomValue(-10, 10)) };
    const Vector2 vel = Vector2Normalize(
      Vector2{ static_cast<float>(GetRandomValue(-100, 100)), static_cast<float>(GetRandomValue(-100, 100)) });
    const Color color{ 250, 200, 120, 240 };
    game.particles->push(Particle::create(pos, vel, color));
  }

  lives--;
  position.x = Game::width / 2.0f;
  position.y = Game::height / 2.0f;
  velocity.x = 0.0f;
  velocity.y = 0.0f;

  invincibility_timer.start();
}

void PlayerShip::handle_input()
{
  auto &game = Game::get();

  if (IsKeyDown(KEY_LEFT))
    sprite.rotation -= 1.0f * rotation_speed;

  if (IsKeyDown(KEY_RIGHT))
    sprite.rotation += 1.0f * rotation_speed;

  const bool is_accelerating = IsKeyDown(KEY_UP);

  if (is_accelerating)
  {
    velocity.x -= cos(sprite.rotation * DEG2RAD + M_PI / 2.0f) * acceleration_speed;
    velocity.y -= sin(sprite.rotation * DEG2RAD + M_PI / 2.0f) * acceleration_speed;

    sprite.set_tag("fly");

    for (int i = 0; i < 1; ++i)
    {
      const Vector2 pos{
        static_cast<float>(position.x + cos(sprite.rotation * DEG2RAD + M_PI / 2.0f) * 10.0f + GetRandomValue(-2, 2)),
        static_cast<float>(position.y + sin(sprite.rotation * DEG2RAD + M_PI / 2.0f) * 10.0f + GetRandomValue(-2, 2))
      };
      Vector2 vel{ static_cast<float>(cos(sprite.rotation * DEG2RAD + M_PI / 2.0f) * 2.0f),
                   static_cast<float>(sin(sprite.rotation * DEG2RAD + M_PI / 2.0f) * 2.0f) };
      Color color = WHITE;
      color.a     = 40;
      game.particles->push(Particle::create(pos, vel, color));

      vel.x *= 0.5f;
      vel.y *= 0.5f;
      color   = BROWN;
      color.a = 80;
      game.particles->push(Particle::create(pos, vel, color));
    }
  }
  else
  {
    sprite.set_tag("idle");
  }

  if (IsKeyDown(KEY_DOWN))
  {
    velocity.x += cos(sprite.rotation * DEG2RAD + M_PI / 2.0f) * acceleration_speed * 0.005f;
    velocity.y += sin(sprite.rotation * DEG2RAD + M_PI / 2.0f) * acceleration_speed * 0.005f;
  }

  if (IsKeyDown(KEY_SPACE) && shoot_timer.is_done() && !is_invincible())
  {
    Vector2 bullet_position;
    Vector2 bullet_velocity;
    bullet_position.x = position.x - cos(sprite.rotation * DEG2RAD + M_PI / 2.0f) * 5.0f;
    bullet_position.y = position.y - sin(sprite.rotation * DEG2RAD + M_PI / 2.0f) * 5.0f;
    bullet_velocity.x = cos(sprite.rotation * DEG2RAD + M_PI / 2.0f + M_PI) * 5.0f;
    bullet_velocity.y = sin(sprite.rotation * DEG2RAD + M_PI / 2.0f + M_PI) * 5.0f;
    bullet_velocity.x += velocity.x * 0.5f;
    bullet_velocity.y += velocity.y * 0.5f;

    BulletType bullet_type = BulletType::Normal;

    if (bullet_type == BulletType::Normal)
    {
      game.bullets->push(Bullet::create_normal(bullet_position, bullet_velocity));
      for (int i = 0; i < 4; ++i)
      {
        const Vector2 pos{
          static_cast<float>(position.x + cos(sprite.rotation * DEG2RAD + M_PI / 2.0f) * 10.0f + GetRandomValue(-2, 2)),
          static_cast<float>(position.y + sin(sprite.rotation * DEG2RAD + M_PI / 2.0f) * 10.0f + GetRandomValue(-2, 2))
        };
        Color color = PINK;
        color.a     = 120;
        game.particles->push(Particle::create(pos, Vector2Scale(bullet_velocity, 0.99f), color));
        color.a = 20;
        game.particles->push(Particle::create(pos, Vector2Scale(bullet_velocity, 0.2f), color));
      }
    }
    else if (bullet_type == BulletType::Homing)
    {
      game.bullets->push(Bullet::create_homing(bullet_position, bullet_velocity));
    }
    else if (bullet_type == BulletType::Assisted)
    {
      game.bullets->push(Bullet::create_assisted(position, bullet_velocity));
    }

    shoot_timer.start();
  }
}

void PlayerShip::update()
{
  invincibility_timer.update(Game::delta_time);

  shoot_timer.update(Game::delta_time);
  handle_input();

  position.x += velocity.x;
  position.y += velocity.y;

  velocity.x *= drag;
  velocity.y *= drag;

  if (Vector2Length(velocity) > max_velocity)
  {
    velocity = Vector2Normalize(velocity);
    velocity.x *= max_velocity;
    velocity.y *= max_velocity;
  }

  wrap_position(position);

  if (!is_invincible())
  {
    GAME.asteroids->for_each(
      [&](Asteroid &asteroid) -> void
      {
        if (asteroid.mask.check_collision(mask))
        {
          die();
          return;
        }
      });
  }

  sprite.animate();
  mask.position = position;
}
