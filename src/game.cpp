#include "game.hpp"

#include <raylib.h>
#include <raymath.h>

#include "asteroid.hpp"
#include "bullet.hpp"
#include "object_circular_buffer.hpp"
#include "player.hpp"

Config Game::CONFIG{};

Game &Game::get() noexcept
{
  static Game game;
  return game;
}

void Game::init()
{
  player    = std::make_unique<Player>();
  bullets   = std::make_unique<ObjectCircularBuffer<Bullet>>();
  asteroids = std::make_unique<ObjectCircularBuffer<Asteroid>>();

  for (size_t i = 0; i < NUMBER_OF_ASTEROIDS; i++)
  {
    const Vector2 position = { static_cast<float>(GetRandomValue(0, width)),
                               static_cast<float>(GetRandomValue(0, height)) };
    asteroids->push(Asteroid::create(position, 2));
  }
}

void Game::update()
{
  player->update();
  bullets->update();
  asteroids->update();
}

void Game::draw() noexcept
{
  draw_background();

  player->draw();
  bullets->draw();
  asteroids->draw();
}

void Game::draw_background() noexcept
{
  auto asteroid_sprite = Sprite{ "resources/asteroid.aseprite" };
  asteroid_sprite.set_frame(1);

  for (int x = 0; x < width; x += asteroid_sprite.get_width())
  {
    for (int y = 0; y < height; y += asteroid_sprite.get_height())
    {
      asteroid_sprite.position = Vector2{ static_cast<float>(x), static_cast<float>(y) };
      asteroid_sprite.tint = ColorBrightness(BLACK, 0.2f);
      asteroid_sprite.draw();
    }
  }
}
