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

  camera.position = Vector3{ static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f, 10.0f };
  camera.target   = Vector3{ static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f, 0.0f };
  camera.up       = Vector3{ 0.0f, 0.0f, 1.0f };
  camera.fovy     = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;
}

void Game::update()
{
  player->update();
  bullets->update();
  asteroids->update();

  camera.position = Vector3 { 0.0f, 20.0f, 0.0f };
  camera.target = Vector3{ player->position.x * 0.1f, player->position.y * 0.1f, 0.0f };
}

void Game::draw() noexcept
{
  BeginMode3D(camera);
  {
    DrawCube(Vector3{ 0.0f, 0.0f, 0.0f }, 0.2f, 0.2f, 0.2f, BLUE);

    player->draw();
    bullets->draw();
    asteroids->draw();

    DrawGrid(100, 10.0f);
    DrawSphere(Vector3{ 0.0f, 0.0f, 0.0f }, 1.0f, RED);

  }
  EndMode3D();
}
