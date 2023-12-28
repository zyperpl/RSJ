#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <functional>
#include <memory>

#if !defined(M_PI)
static constexpr const float M_PI = 3.14159265358979323846f;
#endif

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include "game.hpp"
#include "mask.hpp"
#include "object_circular_buffer.hpp"
#include "sprite.hpp"
#include "timer.hpp"

#define FRAMES(n) (Game::delta_time * (float)(n))

const Color CRYSTAL_COLOR{ 255, 137, 51, 255 };

[[nodiscard]] Rectangle texture_rect(const Texture2D &texture);

[[nodiscard]] Rectangle texture_rect_flipped(const Texture2D &texture);

void wrap_position(Vector2 &position);

void draw_wrapped(const Rectangle &rect, const std::function<void(const Vector2 &)> draw_function);

enum class Direction
{
  Left,
  Right,
  Up,
  Down
};
