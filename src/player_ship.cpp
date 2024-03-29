#include "player_ship.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#include <functional>

#include <raylib.h>
#include <raymath.h>

#include "asteroid.hpp"
#include "bullet.hpp"
#include "game.hpp"
#include "interactable.hpp"
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
  if (invincibility_timer.get_ratio() < 0.7f)
  {
    const int invincibility_timer_int = static_cast<int>(invincibility_timer.get_remaining_time() * 10.0f);
    if (!invincibility_timer.is_done() && invincibility_timer_int % 2 == 0)
      return;
  }

  sprite.set_centered();

  sprite.position = position;

  draw_wrapped(sprite.get_destination_rect(),
               [&](const Vector2 &P)
               {
                 sprite.position = P;
                 sprite.draw();

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

  sound_explode.play();

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

  if (game.score > 1000)
    game.score -= 1000;

  if (lives <= 0)
  {
    sound_shoot.play();
    sound_engine.stop();

    if (game.score > 10000)
      game.score -= 10000;

    const size_t r = static_cast<size_t>(GetRandomValue(1, 5));

    if (game.crystals >= r)
    {
      game.crystals -= r;
      game.gui->show_message("You have died! You lost " + std::to_string(r) + " crystals.");
    }
    else
    {
      game.gui->show_message("You have died!");
    }

    lives = max_lives;
  }

  invincibility_timer.start();
  invincibility_timer.update();
}

BulletType bullet_type_from_gun(GunType gun) noexcept
{
  switch (gun)
  {
    case GunType::Normal:
      return BulletType::Normal;
    case GunType::Fast:
      return BulletType::Normal;
    case GunType::Assisted:
      return BulletType::Assisted;
    case GunType::Homing:
      return BulletType::Homing;
  }
  return BulletType::Normal;
}

void PlayerShip::shoot() noexcept
{
  auto &game = Game::get();

  sound_shoot.play();

  float bullet_speed = 3.0f;
  if (game.gun == GunType::Fast)
    bullet_speed = 6.0f;

  Vector2 bullet_position;
  Vector2 bullet_velocity;
  bullet_position.x = position.x - cos(sprite.rotation * DEG2RAD + M_PI / 2.0f) * 5.0f;
  bullet_position.y = position.y - sin(sprite.rotation * DEG2RAD + M_PI / 2.0f) * 5.0f;
  bullet_velocity.x = cos(sprite.rotation * DEG2RAD + M_PI / 2.0f + M_PI) * bullet_speed;
  bullet_velocity.y = sin(sprite.rotation * DEG2RAD + M_PI / 2.0f + M_PI) * bullet_speed;
  bullet_velocity.x += velocity.x * 0.5f;
  bullet_velocity.y += velocity.y * 0.5f;

  BulletType bullet_type = bullet_type_from_gun(game.gun);

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

  if (game.gun == GunType::Normal)
    shoot_timer.set_max_time(FRAMES(30));
  else
    shoot_timer.set_max_time(FRAMES(20));

  shoot_timer.start();
}

bool PlayerShip::can_shoot() const noexcept
{
  return shoot_timer.is_done() && invincibility_timer.get_ratio() >= 0.3f && !interactable;
}

bool PlayerShip::can_interact() const noexcept
{
  return !is_interacting && interactable;
}

void PlayerShip::handle_input()
{
  auto &game = Game::get();
  if (game.get_state() != GameState::PLAYING_ASTEROIDS)
    return;

  if (game.input.left_held())
    sprite.rotation -= 1.0f * rotation_speed;

  if (game.input.right_held())
    sprite.rotation += 1.0f * rotation_speed;

  const bool is_accelerating = game.input.up_held();

  if (is_accelerating)
  {
    velocity.x -= cos(sprite.rotation * DEG2RAD + M_PI / 2.0f) * acceleration_speed;
    velocity.y -= sin(sprite.rotation * DEG2RAD + M_PI / 2.0f) * acceleration_speed;

    if (!sound_engine.is_playing())
      sound_engine.play();

    if (sound_engine.volume.value_or(0.0f) < 0.5f)
      sound_engine.set_volume(Lerp(sound_engine.volume.value_or(0.0f), 0.5f, 0.1f));

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
    if (sound_engine.is_playing())
    {
      if (sound_engine.volume)
        sound_engine.set_volume(Lerp(sound_engine.volume.value(), 0.0f, 0.1f));

      if (sound_engine.volume < 0.01f)
      {
        sound_engine.stop();
        sound_engine.set_volume(0.0f);
      }
    }

    sprite.set_tag("idle");
  }

  if (game.input.down_held())
  {
    velocity.x += cos(sprite.rotation * DEG2RAD + M_PI / 2.0f) * acceleration_speed * 0.007f;
    velocity.y += sin(sprite.rotation * DEG2RAD + M_PI / 2.0f) * acceleration_speed * 0.007f;
  }

  if (game.input.action_held() && can_shoot())
    shoot();

  if (game.input.action_pressed() && can_interact() && interactable)
  {
    sound_shoot.stop();
    sound_engine.stop();
    sound_explode.stop();

    interactable->interact();
    interactable   = nullptr;
    is_interacting = true;
  }
}

void PlayerShip::update()
{
  auto &game = Game::get();
  if (game.get_state() != GameState::PLAYING_ASTEROIDS)
    return;

  // timers
  invincibility_timer.update();
  shoot_timer.update();

  // movement
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
  find_nearest_interactive();

  // logic
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

  // sprite and mask
  sprite.animate();
  mask.position = position;
}

void PlayerShip::find_nearest_interactive() noexcept
{
  interactable = nullptr;
  auto &game   = Game::get();

  const bool no_asteroids = game.asteroids->empty();

  for (auto &obj : game.room->interactables)
  {
    const float min_distance       = std::max(obj->get_sprite().get_width(), obj->get_sprite().get_height()) * 0.5f;
    const float distance           = Vector2Distance(position, obj->get_sprite().position);
    Interactable *interactable_ptr = obj.get();
    if (distance < min_distance && no_asteroids && dynamic_cast<Station *>(interactable_ptr))
    {
      // HACK: hack to make the station interactable only when big enough
      if (!is_interacting && interactable_ptr->get_sprite().scale.x > 0.6f)
      {
        interactable = interactable_ptr;
        break;
      }
    }
  }
}
