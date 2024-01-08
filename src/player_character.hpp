#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>

#include <raylib.h>
#include <raymath.h>

#include "mask.hpp"
#include "player.hpp"
#include "sound_manager.hpp"
#include "sprite.hpp"
#include "timer.hpp"
#include "utils.hpp"

class Interactable;

class PlayerCharacter final : public Player
{
public:
  PlayerCharacter();
  mutable Sprite sprite{ "resources/character.aseprite", "idle_down" };

  void handle_input() override;
  void update() override;
  void draw() const noexcept override;
  void die() override;

  void animate();

  bool is_colliding() const;

private:
  Direction direction{ Direction::Down };

  constexpr static float PLAYER_SPEED = 2.0f;

  SMSound sound_step = SoundManager::copy("resources/step.wav");
};
