#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>
#include <optional>

#include <raylib.h>
#include <raymath.h>

#include "interactable.hpp"
#include "mask.hpp"
#include "sprite.hpp"
#include "timer.hpp"
#include "utils.hpp"

class Player
{
public:
  virtual ~Player() = default;

  Vector2 position{ 240.0f, 160.0f };
  Vector2 velocity{ 0.0f, 0.0f };
  Mask mask{};

  int max_lives{ 3 };
  int lives{ max_lives };

  virtual void handle_input()        = 0;
  virtual void update()              = 0;
  virtual void draw() const noexcept = 0;
  virtual void die()                 = 0;

  virtual bool can_interact() const noexcept { return interactable; }

  const Mask &get_mask() const noexcept { return mask; }

  const Interactable *get_interactable() const noexcept { return interactable; }

protected:
  float rotation_speed{ 3.0f };
  float acceleration_speed{ 0.04f };
  float max_velocity{ 60.0f };
  float drag{ 0.99f };
  Interactable *interactable{ nullptr };
};
