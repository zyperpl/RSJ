#pragma once

#include <memory>
#include <optional>

#include <raylib.h>
#include <raymath.h>

#include "dialog.hpp"

class Sprite;

class GUI
{
public:
  GUI();
  ~GUI();

  void update();
  void draw() const noexcept;

  bool is_active() const;

  void set_dialog(const Dialog &new_dialog) noexcept;
  void reset_dialog() noexcept;
  void draw_dialog() const noexcept;

  void draw_shop_items(const std::vector<ShopItem> &items) const noexcept;

  void handle_selecting_index(std::optional<size_t> &index, size_t max_index) const noexcept;
  void handle_accepting_index(std::optional<size_t> &index, std::function<void(size_t)> func) const noexcept;

private:
  std::unique_ptr<Sprite> ui_crystal;

  std::optional<Dialog> dialog;
  std::optional<size_t> selected_index;

  Font font;
  Font dialog_font;

  const float font_size{ 10.0f };

  friend class Game;
};
