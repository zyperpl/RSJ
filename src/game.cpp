#include "game.hpp"

#include <raylib.h>
#include <raymath.h>

#include "asteroid.hpp"
#include "bullet.hpp"
#include "object_circular_buffer.hpp"
#include "particle.hpp"
#include "player.hpp"
#include "utils.hpp"

Config Game::CONFIG{};

Game &Game::get() noexcept
{
  static Game game;
  return game;
}

void Game::init()
{
  player    = std::make_unique<Player>();
  bullets   = std::make_unique<ObjectCircularBuffer<Bullet, 64>>();
  asteroids = std::make_unique<ObjectCircularBuffer<Asteroid, 128>>();
  particles = std::make_unique<ObjectCircularBuffer<Particle, 1024>>();

  for (size_t i = 0; i < NUMBER_OF_ASTEROIDS; i++)
  {
    const Vector2 position = { static_cast<float>(GetRandomValue(0, width)),
                               static_cast<float>(GetRandomValue(0, height)) };
    asteroids->push(Asteroid::create(position, 2));
  }

  for (size_t i = 0; i < 10; i++)
  {
    Particle particle;
    particle.position =
      Vector2{ static_cast<float>(GetRandomValue(0, width)), static_cast<float>(GetRandomValue(0, height)) };
    particle.velocity = Vector2{ static_cast<float>(GetRandomValue(-100, 100)) / 100.0f,
                                 static_cast<float>(GetRandomValue(-100, 100)) / 100.0f };
    particle.color    = Color{ static_cast<unsigned char>(GetRandomValue(0, 255)),
                            static_cast<unsigned char>(GetRandomValue(0, 255)),
                            static_cast<unsigned char>(GetRandomValue(0, 255)),
                            static_cast<unsigned char>(GetRandomValue(0, 255)) };
    particles->push(std::move(particle));
  }

  for (size_t i = 0; i < stars.size(); i++)
  {
    stars[i] = Vector2{ static_cast<float>(GetRandomValue(0, width)), static_cast<float>(GetRandomValue(0, height)) };
  }
}

void Game::update()
{
  player->update();
  bullets->for_each(std::bind(&Bullet::update, std::placeholders::_1));
  asteroids->for_each(std::bind(&Asteroid::update, std::placeholders::_1));
  particles->for_each(std::bind(&Particle::update, std::placeholders::_1));

  for (size_t i = 0; i < stars.size(); i++)
  {
    stars[i].x += 0.1f;
    if (i % 2 == 0)
      stars[i].x += 0.2f;

    if (stars[i].x > width)
    {
      stars[i].x -= width;
      stars[i].y = static_cast<float>(GetRandomValue(0, height));
    }
  }
}

void Game::draw() noexcept
{
  draw_background();
  particles->for_each(std::bind(&Particle::draw, std::placeholders::_1));

  player->draw();
  bullets->for_each(std::bind(&Bullet::draw, std::placeholders::_1));
  asteroids->for_each(std::bind(&Asteroid::draw, std::placeholders::_1));
}

void Game::draw_background() noexcept
{
  static int frame = 0;
  frame++;

  for (size_t i = 0; i < stars.size(); i++)
  {
    const Vector2 &star = stars[i];
    if (i % 2 == 0)
      DrawPixel(star.x, star.y, Color{ 180, 180, 100, 255 });
    else
      DrawPixel(star.x, star.y, Color{ 120, 120, 100, 255 });
  }

  auto asteroid_sprite = Sprite{ "resources/asteroid.aseprite" };
  asteroid_sprite.set_frame(1);
  asteroid_sprite.tint = ColorBrightness(BLACK, 0.2f);
  const long &w        = static_cast<long>(asteroid_sprite.get_width());
  const long &h        = static_cast<long>(asteroid_sprite.get_height());

  for (int x = -w; x <= width + w; x += asteroid_sprite.get_width())
  {
    for (int y = -h; y <= height + h; y += asteroid_sprite.get_height())
    {
      if ((x * y) % 3 == 0 || (x + y) % 5 == 0 || (x * y) % 7 == 0 || (x + y) % 9 == 0)
        continue;

      float xf                 = static_cast<float>(x) - sin(frame * 0.001f + x * 37) * static_cast<float>(w) * 0.4f;
      float yf                 = static_cast<float>(y) - cos(frame * 0.002f - y * 13) * static_cast<float>(h) * 0.5f;
      asteroid_sprite.position = Vector2{ xf, yf };
      asteroid_sprite.set_frame((x + y - 1) % 3);
      asteroid_sprite.draw();
    }
  }
}
