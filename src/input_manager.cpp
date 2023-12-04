#include "input_manager.hpp"

InputManager::InputManager()
{
  reset();
}

void InputManager::update()
{
  // menu input
  if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_ENTER))
    set_action_key_active(ActionKey::MenuEnter);

  if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE))
    set_action_key_active(ActionKey::MenuBack);

  if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
    set_action_key_active(ActionKey::MenuDown);

  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
    set_action_key_active(ActionKey::MenuUp);

  if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
    set_action_key_active(ActionKey::MenuLeft);

  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
    set_action_key_active(ActionKey::MenuRight);

  if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P))
    set_action_key_active(ActionKey::Pause);

  // player input
  if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
    set_action_key_active(ActionKey::Left);

  if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
    set_action_key_active(ActionKey::Right);

  if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_T) || IsKeyPressed(KEY_UP))
    set_action_key_active(ActionKey::Up);

  if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
    set_action_key_active(ActionKey::Down);

  if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_T) || IsKeyPressed(KEY_UP))
    set_action_key_active(ActionKey::Jump);

  if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W) || IsKeyDown(KEY_T) || IsKeyDown(KEY_UP))
    set_action_key_active(ActionKey::HoldJump);

  if (IsKeyPressed(KEY_X) || IsKeyPressed(KEY_G) || IsKeyPressed(KEY_E) || IsKeyPressed(KEY_LEFT_SHIFT) ||
      IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    set_action_key_active(ActionKey::Action);

  if (IsKeyDown(KEY_Z) || IsKeyDown(KEY_C) || IsKeyDown(KEY_K) || IsKeyDown(KEY_B) || IsKeyDown(KEY_Q) ||
      IsKeyDown(KEY_F) || IsKeyDown(KEY_LEFT_CONTROL) || IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    set_action_key_active(ActionKey::Shoot);

  // mouse buttons
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    set_action_mouse_button_active(ActionMouseButton::LeftClick);
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    set_action_mouse_button_active(ActionMouseButton::LeftDrag);

  if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
    set_action_mouse_button_active(ActionMouseButton::MiddleClick);
  if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
    set_action_mouse_button_active(ActionMouseButton::MiddleDrag);

  if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    set_action_mouse_button_active(ActionMouseButton::RightClick);
  if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    set_action_mouse_button_active(ActionMouseButton::RightDrag);
}

void InputManager::reset()
{
  for (auto &k : action_keys)
    k = false;

  for (auto &mb : action_mouse_buttons)
    mb = false;
}

bool InputManager::is_active(ActionKey k)
{
  return get_action_key(k);
}

bool InputManager::is_active(ActionMouseButton mb)
{
  return get_action_mouse_button(mb);
}

void InputManager::set_inactive(ActionKey k)
{
  action_keys[static_cast<long>(k)] = false;
}

void InputManager::set_inactive(ActionMouseButton mb)
{
  action_mouse_buttons[static_cast<long>(mb)] = false;
}
