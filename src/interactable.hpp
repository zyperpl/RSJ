#pragma once

#include "dialog.hpp"
#include "mask.hpp"
#include "sprite.hpp"
#include "utils.hpp"

class Interactable
{
public:
  virtual ~Interactable() = default;

  virtual void update(){};
  virtual void draw() const;
  virtual void interact() = 0;

  [[nodiscard]] const Sprite &get_sprite() const noexcept { return sprite; }

  [[nodiscard]] virtual bool is_interactable() const { return true; }

protected:
  mutable Sprite sprite{};
};

class Station final : public Interactable
{
public:
  Station();
  void update() override;
  void interact() override;
};

class DockedShip final : public Interactable
{
public:
  DockedShip();
  void interact() override;
};

class DialogEntity final : public Interactable
{
public:
  DialogEntity(const Vector2 &position, const std::string &name);
  void update() override;
  void interact() override;

  [[nodiscard]] const Dialog &dialog() const { return get_dialog(dialog_id); }

  void set_dialog_id(const DialogId &id) { dialog_id = id; }
  const std::string &get_dialog_id() const { return dialog_id; }

  void reset_animation() { sprite.set_animation(default_animation_tag); }

  [[nodiscard]] bool is_interactable() const override { return !dialogs.empty(); }

private:
  [[nodiscard]] const Dialog &get_dialog(const DialogId &dialog_id) const
  {
    if (dialog_id == Dialog::END_DIALOG_ID)
      return Dialog::END_DIALOG;

    assert(dialogs.contains(dialog_id));

    auto &dialog = dialogs.at(dialog_id);
    if (dialog.prefunc)
    {
      const auto next_dialog_id = dialog.prefunc();
      assert(next_dialog_id != dialog_id);
      if (next_dialog_id)
        return get_dialog(*next_dialog_id);
    }

    return dialog;
  }

  DialogMap dialogs;
  DialogId dialog_id{ Dialog::START_DIALOG_ID };

  std::string default_animation_tag{ "idle_down" };
};
