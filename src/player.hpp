#pragma once

#include "game_object.hpp"
#include "animation_sprite.hpp"

#include <raymath.h>

#include "timer.hpp"
#include "sound_manager.hpp"

class InputManager;

class Player final : public GameObject
{
public:
  Player();

  void draw(const RenderPass &render_pass) override;
  void step() override;

  void process_input(InputManager &input);
private:
};
