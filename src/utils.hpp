#pragma once

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include "game.hpp"
#include "object_circular_buffer.hpp"

#define FRAMES(n) (Game::delta_time * (float)(n))

[[nodiscard]] Rectangle texture_rect(const Texture2D &texture);

[[nodiscard]] Rectangle texture_rect_flipped(const Texture2D &texture);

void wrap_position(Vector2 &position);
