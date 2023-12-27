#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>

#include <raylib.h>
#include <raymath.h>

#include "mask.hpp"
#include "sprite.hpp"
#include "timer.hpp"
#include "utils.hpp"

#include "player.hpp"

class PlayerShip final : public Player
{
public:
  PlayerShip();
  mutable Sprite sprite{ "resources/ship.aseprite", "idle" };

  void handle_input() override;

  void update() override;

  void draw() const noexcept override;
  void die() override;

private:
  enum class InteractiveType
  {
    NONE,
    STATION
  };

  Timer shoot_timer{ FRAMES(20) };
  Timer invincibility_timer{ FRAMES(150) };

  [[nodiscard]] bool is_invincible() const noexcept { return !invincibility_timer.is_done(); }

  void find_nearest_interactive() noexcept;
  Interactable *interactable{ nullptr };
  Timer interactive_found_timer{ FRAMES(4) };

  void shoot() noexcept;

  bool can_shoot() const noexcept;
  bool can_interact() const noexcept;
};
