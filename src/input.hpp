#pragma once

#include <cstdint>
#include <string_view>
#include <unordered_map>

#include <raylib.h>

#define INPUT_ACTION_LIST  \
  INPUT_ACTION(up)         \
  INPUT_ACTION(down)       \
  INPUT_ACTION(left)       \
  INPUT_ACTION(right)      \
  INPUT_ACTION(action)     \
  INPUT_ACTION(mute)       \
  INPUT_ACTION(menu_up)    \
  INPUT_ACTION(menu_down)  \
  INPUT_ACTION(menu_left)  \
  INPUT_ACTION(menu_right) \
  INPUT_ACTION(menu_action)

#define KEY_STATE_NOT_PRESSED 0
#define KEY_STATE_PRESSED     1
#define KEY_STATE_HELD        2
#define KEY_STATE_RELEASED    3

class Input
{
public:
  void update();
  void gather();

  [[deprecated("Use `update` and `gather`")]] void reset();

#undef INPUT_ACTION
#define INPUT_ACTION(name)                                                                \
  [[nodiscard]] bool name##_held() const                                                  \
  {                                                                                       \
    return name##_state >= KEY_STATE_PRESSED && name##_state < KEY_STATE_RELEASED;        \
  }                                                                                       \
  [[nodiscard]] bool name##_pressed() const { return name##_state == KEY_STATE_PRESSED; } \
  [[nodiscard]] bool name##_released() const { return name##_state == KEY_STATE_RELEASED; }

  INPUT_ACTION_LIST

#if defined(DEBUG)
  void debug_draw() const
  {
    int y = 0;
    static const std::unordered_map<uint8_t, Color> colors{
      { KEY_STATE_NOT_PRESSED, GRAY },
      { KEY_STATE_PRESSED, RED },
      { KEY_STATE_HELD, GREEN },
      { KEY_STATE_RELEASED, BLUE },
    };

    static const std::unordered_map<uint8_t, std::string_view> strings{
      { KEY_STATE_NOT_PRESSED, "not pressed" },
      { KEY_STATE_PRESSED, "Pressed" },
      { KEY_STATE_HELD, "HELD" },
      { KEY_STATE_RELEASED, "released" },
    };
#undef INPUT_ACTION
#define INPUT_ACTION(name)                                                                                      \
  DrawText(TextFormat("%10s:%10s", #name, strings.at(name##_state).data()), 0, y, 10, colors.at(name##_state)); \
  y += 12;

    INPUT_ACTION_LIST
  }
#endif

private:
#undef INPUT_ACTION
#define INPUT_ACTION(name) mutable uint8_t name##_state{ KEY_STATE_NOT_PRESSED };

  INPUT_ACTION_LIST
};
