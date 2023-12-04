#pragma once

#include <algorithm>
#include <cmath>

class Timer
{
public:
  constexpr Timer() = delete;

  constexpr Timer(float max_time_value)
    : max_time{ max_time_value }
    , remaining_time{ max_time }
  {
  }

  constexpr Timer(float max_time_value, float current_remaining_time)
    : max_time{ max_time_value }
    , remaining_time{ current_remaining_time }
  {
  }

  void set_max_time(float new_time) { max_time = new_time; }

  inline void start() { remaining_time = max_time; }

  inline void stop() { remaining_time = -0.0f; }

  inline void update(float delta_time)
  {
    if (remaining_time <= 0.0f)
      return;

    remaining_time -= delta_time;
  }

  [[nodiscard]] inline bool is_done() const { return remaining_time <= std::numeric_limits<float>::epsilon(); }
  [[nodiscard]] inline float get_remaining_time() const { return remaining_time; }
  [[nodiscard]] inline bool has_recently_started(float delta_time, float steps) const
  {
    const float time_offset = steps * delta_time;
    return !is_done() && remaining_time >= max_time - time_offset;
  }

  [[nodiscard]] inline float get_ratio() const { return 1.0f - std::clamp(remaining_time / max_time, 0.0f, 1.0f); }

private:
  float max_time;
  float remaining_time;
};
