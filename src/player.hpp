#pragma once

#include <memory>

#include <raylib.h>
#include <raymath.h>

#include "sprite.hpp"
#include "timer.hpp"
#include "utils.hpp"

class Player
{
public:
  Sprite sprite{ "resources/ship.aseprite", "idle" };
  Vector2 position{ Game::width / 2.0f, Game::height / 2.0f };
  Vector2 velocity{ 0.0f, 0.0f };

  float rotation_speed{ 3.0f };
  float acceleration_speed{ 0.04f };
  float max_velocity{ 40.0f };
  float drag{ 0.99f };

  int lives{ 3 };

  Timer shoot_timer{ FRAMES(20) };
  Timer invincibility_timer{ FRAMES(180) };

  void handle_input();

  void update();

  void draw() noexcept;
  void die();

  [[nodiscard]] bool is_invincible() const noexcept { return !invincibility_timer.is_done(); }
};
