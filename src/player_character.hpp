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

class PlayerCharacter final : public Player
{
public:
  PlayerCharacter();
  mutable Sprite sprite{ "resources/character.aseprite", "idle_down" };

  void handle_input() override;
  void update() override;
  void draw() const noexcept override;
  void die() override;
private:
};
