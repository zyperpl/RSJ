#include "player.hpp"

#include <cmath>
#include <functional>

#include "asteroid.hpp"
#include "bullet.hpp"
#include "game.hpp"
#include "utils.hpp"

void Player::draw() const noexcept
{
  const int invincibility_timer_int = static_cast<int>(invincibility_timer.get_remaining_time() * 10.0f);
  if (!invincibility_timer.is_done() && invincibility_timer_int % 2 == 0)
    return;

  sprite.set_centered();

  sprite.position = position;

  draw_wrapped(
    sprite.get_rect(),
    [&](const Vector2 &P)
    {
      sprite.position = P;
      sprite.draw();

      DrawCircleV(P, 1.0f, GREEN);

      if (Game::CONFIG.show_masks)
      {
        Mask mask_copy     = mask;
        mask_copy.position = P;
        mask_copy.draw();
      }

      if (Game::CONFIG.show_velocity)
        DrawLineEx(position, Vector2{ position.x + velocity.x * 10.0f, position.y + velocity.y * 10.0f }, 1.0f, RED);

      if (Game::CONFIG.show_acceleration)
      {
        DrawLineEx(
          position,
          Vector2{ static_cast<float>(position.x + cos(sprite.rotation * DEG2RAD + M_PI / 2.0f) * acceleration_speed),
                   static_cast<float>(position.y + sin(sprite.rotation * DEG2RAD + M_PI / 2.0f) * acceleration_speed) },
          1.0f,
          RED);
      }
    });
}

void Player::die()
{
  lives--;
  position.x = Game::width / 2.0f;
  position.y = Game::height / 2.0f;
  velocity.x = 0.0f;
  velocity.y = 0.0f;

  invincibility_timer.start();
}

void Player::handle_input()
{
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
    Bullet bullet;
    bullet.position   = position;
    bullet.velocity.x = cos(sprite.rotation * DEG2RAD + M_PI / 2.0f + M_PI) * 5.0f;
    bullet.velocity.y = sin(sprite.rotation * DEG2RAD + M_PI / 2.0f + M_PI) * 5.0f;
    Game::get().bullets->push(std::move(bullet));

    shoot_timer.start();
  }
}

void Player::update()
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
    for (size_t i = Game::get().asteroids->tail; i < Game::get().asteroids->head; i++)
    {
      Asteroid &asteroid = Game::get().asteroids->objects[i];
      if (asteroid.mask.check_collision(mask))
      {
        die();
        break;
      }
    }
  }

  sprite.animate();
  mask.position = position;
}
