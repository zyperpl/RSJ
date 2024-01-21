#include "input.hpp"

#include <raylib.h>

void Input::update()
{
  static const constexpr int up_keys[]{ KEY_UP, KEY_W, KEY_KP_8 };
  static const constexpr int down_keys[]{ KEY_DOWN, KEY_S, KEY_KP_2 };
  static const constexpr int left_keys[]{ KEY_LEFT, KEY_A, KEY_KP_4 };
  static const constexpr int right_keys[]{ KEY_RIGHT, KEY_D, KEY_KP_6 };
  static const constexpr int action_keys[]{ KEY_SPACE, KEY_ENTER, KEY_KP_ENTER, KEY_Z, KEY_F, KEY_X };
  static const constexpr int mute_keys[]{ KEY_M };
  static const constexpr int menu_up_keys[]{ KEY_UP, KEY_W, KEY_KP_8 };
  static const constexpr int menu_down_keys[]{ KEY_DOWN, KEY_S, KEY_KP_2 };
  static const constexpr int menu_left_keys[]{ KEY_LEFT, KEY_A, KEY_KP_4 };
  static const constexpr int menu_right_keys[]{ KEY_RIGHT, KEY_D, KEY_KP_6 };
  static const constexpr int menu_action_keys[]{ KEY_SPACE, KEY_ENTER, KEY_KP_ENTER, KEY_Z, KEY_F, KEY_X };

  static const constexpr auto update_key_state = [](uint8_t &state, const int &key) -> bool
  {
    const bool is_down = IsKeyDown(key);
    if (is_down && state != KEY_STATE_PRESSED && state != KEY_STATE_HELD)
    {
      state = KEY_STATE_PRESSED;
      return true;
    }
    else if (is_down)
    {
      state = KEY_STATE_HELD;
      return true;
    }
    else if (!is_down && state != KEY_STATE_NOT_PRESSED)
    {
      state = KEY_STATE_RELEASED;
      return true;
    }

    return false;
  };

#undef INPUT_ACTION
#define INPUT_ACTION(name)                   \
  for (const int key : name##_keys)          \
  {                                          \
    if (update_key_state(name##_state, key)) \
      break;                                 \
  }

  INPUT_ACTION_LIST
}

void Input::reset()
{
#undef INPUT_ACTION
#define INPUT_ACTION(name) name##_state = KEY_STATE_NOT_PRESSED;

  INPUT_ACTION_LIST
}
