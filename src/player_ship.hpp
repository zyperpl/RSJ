#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>

#include <raylib.h>
#include <raymath.h>

#include "mask.hpp"
#include "sound_manager.hpp"
#include "sprite.hpp"
#include "timer.hpp"
#include "utils.hpp"

#include "player.hpp"

class Interactable;

class PlayerShip final : public Player
{
public:
  PlayerShip();
  mutable Sprite sprite{ "resources/ship.aseprite", "idle" };

  void handle_input() override;

  void update() override;

  void draw() const noexcept override;
  void die() override;

  bool can_interact() const noexcept override;

private:
  enum class InteractiveType
  {
    NONE,
    STATION
  };

  Timer shoot_timer{ FRAMES(20) };
  Timer invincibility_timer{ FRAMES(250) };
  bool is_interacting{ false };

  [[nodiscard]] bool is_invincible() const noexcept { return !invincibility_timer.is_done(); }

  void find_nearest_interactive() noexcept;

  void shoot() noexcept;

  bool can_shoot() const noexcept;

  SMSound sound_shoot{ SoundManager::get("resources/shoot.wav") };
  SMSound sound_engine{ SoundManager::copy("resources/engine.wav") };
  SMSound sound_explode{ SoundManager::copy("resources/explosion.wav") };
};
