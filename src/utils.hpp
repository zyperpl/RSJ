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

#include "object_circular_buffer.hpp"
#include "sprite.hpp"

inline constexpr float DELTA_TIME = 1.0f / 60.0f;
[[nodiscard]] inline consteval float FRAMES(auto n)
{
  return (DELTA_TIME * (float)(n));
}

inline constexpr const Color CRYSTAL_COLOR{ 255, 137, 51, 255 };
inline constexpr const char *RESOURCE_PATH{ "resources/" };
[[nodiscard]] inline std::string get_resource_path(const std::string &file_name)
{
  return RESOURCE_PATH + file_name;
}

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

[[nodiscard]] std::string idle_tag_from_direction(const Direction &direction);

[[nodiscard]] std::string walk_tag_from_direction(const Direction &direction);
