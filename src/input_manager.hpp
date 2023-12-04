#pragma once

#include <raylib.h>

enum class ActionKey
{
  Left,
  Right,
  Up,
  Down,
  Jump,
  HoldJump,
  Shoot,
  Action,
  MenuLeft,
  MenuRight,
  MenuUp,
  MenuDown,
  MenuEnter,
  MenuBack,
  Pause,
  ACTION_KEY_MAX
};

enum class ActionMouseButton
{
  LeftClick,
  MiddleClick,
  RightClick,
  LeftDrag,
  MiddleDrag,
  RightDrag,
  ACTION_MOUSE_BUTTON_MAX
};

class InputManager
{
public:
  InputManager();
  void update();
  void reset();
  bool is_active(ActionKey k);
  bool is_active(ActionMouseButton mb);

  void set_inactive(ActionKey k);
  void set_inactive(ActionMouseButton mb);
private:
  bool action_keys[static_cast<long>(ActionKey::ACTION_KEY_MAX)];
  bool action_mouse_buttons[static_cast<long>(ActionMouseButton::ACTION_MOUSE_BUTTON_MAX)];

  bool &get_action_key(ActionKey k) { return action_keys[static_cast<long>(k)]; }
  void set_action_key_active(ActionKey k) { action_keys[static_cast<long>(k)] = true; }

  bool &get_action_mouse_button(ActionMouseButton mb) { return action_mouse_buttons[static_cast<long>(mb)]; }
  void set_action_mouse_button_active(ActionMouseButton mb) { action_mouse_buttons[static_cast<long>(mb)] = true; }
  
};
