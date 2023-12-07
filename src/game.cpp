#include "game.hpp"

#include <raylib.h>
#include <raymath.h>

#include "asteroid.hpp"
#include "bullet.hpp"
#include "object_circular_buffer.hpp"
#include "player.hpp"

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
    Asteroid asteroid;
    asteroid.position.x = GetRandomValue(0, width);
    asteroid.position.y = GetRandomValue(0, height);
    asteroid.velocity.x = static_cast<float>(GetRandomValue(-1, 1)) * 0.5f;
    asteroid.velocity.y = static_cast<float>(GetRandomValue(-1, 1)) * 0.5f;
    asteroids->push(asteroid);
  }

  camera.target   = Vector2{ static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f };
  camera.offset   = Vector2{ static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f };
  camera.zoom     = 1.0f;
  camera.rotation = 0.0f;
}

void Game::update()
{
  player->update();
  bullets->update();
  asteroids->update();

  camera.target   = player->position;
  camera.rotation = -player->rotation;
}

void Game::draw() noexcept
{
  BeginMode2D(camera);
  {
    player->draw();
    bullets->draw();
    asteroids->draw();
  }
  EndMode2D();
}
