#include "gui.hpp"

#include <cmath>
#include <functional>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include "magic_enum/magic_enum.hpp"

#include <raylib.h>
#include <raymath.h>

#include "asteroid.hpp"
#include "dialog.hpp"
#include "player.hpp"
#include "quest.hpp"
#include "room.hpp"
#include "sprite.hpp"
#include "utils.hpp"

#include "game.hpp"

Color selection_color()
{
  const float t         = static_cast<float>(GetTime());
  const unsigned char r = 120 + (sin(t * 6.0f) * 0.5f + 0.5f) * 100;
  const unsigned char g = 120 + (sin(t * 6.0f + 2.0f) * 0.5f + 0.5f) * 100;
  return Color{ r, g, 255, 255 };
}

GUI::GUI()
{
  font        = LoadFontEx("resources/Kenney Mini Square.ttf", 10, nullptr, 0);
  dialog_font = LoadFontEx("resources/Kenney Mini.ttf", 10, nullptr, 0);

  ui_crystal = std::make_unique<Sprite>("resources/ore.aseprite");
}

GUI::~GUI()
{
  UnloadFont(font);
  UnloadFont(dialog_font);
}

void GUI::draw() const noexcept
{
  const Game &game = Game::get();

  Vector2 text_position{ 10.0f, 10.0f };
  DrawTextEx(font, TextFormat("FPS: %i", GetFPS()), text_position, font_size, 1.0f, WHITE);

  text_position.y += font_size + 5.0f;
  const char *lives_text = "Lives: ";
  DrawTextEx(font, lives_text, text_position, font_size, 1.0f, WHITE);
  Vector2 text_size = MeasureTextEx(font, lives_text, font_size, 1.0f);
  float x           = text_position.x + text_size.x + 5.0f;
  float y           = text_position.y + text_size.y * 0.5f;
  for (int i = 0; i < game.player->lives; i++)
  {
    DrawCircle(x + i * 20, y, 5, RED);
  }

  text_position.y += font_size + 5.0f;
  static uint64_t draw_score = 0;
  uint64_t score_step        = std::max<uint64_t>(1, (game.score - draw_score) / 20);
  if (draw_score < game.score)
    draw_score += score_step;
  if (draw_score > game.score)
    draw_score = game.score;
  DrawTextEx(font, TextFormat("Score: %i", draw_score), text_position, font_size, 1.0f, WHITE);

  text_position.y += font_size + 5.0f;
  const char *crystals_text = TextFormat("Crystals: %i", game.crystals);
  DrawTextEx(font, crystals_text, text_position, font_size, 1.0f, WHITE);
  text_size = MeasureTextEx(font, crystals_text, font_size, 1.0f);
  assert(ui_crystal);
  ui_crystal->set_frame(0);
  ui_crystal->scale = Vector2{ 0.4f, 0.4f };
  ui_crystal->set_centered();
  ui_crystal->position.x = text_position.x + text_size.x + 5.0f;
  ui_crystal->position.y = text_position.y + text_size.y * 0.5f;
  ui_crystal->draw();

  // draw quests
  {
    const float quest_right_margin = 10.0f;
    float quest_y                  = 10.0f;
    for (const auto &[quest_name, quest] : game.quests)
    {
      if (!quest.is_accepted() || quest.is_reported())
        continue;

      const auto &quest_text =
        TextFormat("%s: %i/%i", quest.description.c_str(), quest.progress(), quest.max_progress());
      const float quest_x = Game::width - MeasureTextEx(font, quest_text, font_size, 1.0f).x - quest_right_margin;
      const Color color   = quest.is_completed() ? LIME : WHITE;
      DrawTextEx(font, quest_text, Vector2{ quest_x, quest_y }, font_size, 1.0f, color);
      quest_y += font_size + 5.0f;
    }
  }
}

void GUI::set_dialog(const Dialog &new_dialog) noexcept
{
  dialog = new_dialog;
  selected_index.reset();

  if (!dialog->responses.empty())
    selected_index = 0;
}

void GUI::reset_dialog() noexcept
{
  dialog.reset();
  selected_index.reset();
}

void GUI::draw_dialog() const noexcept
{
  if (!dialog)
    return;

  // background
  const float dialog_width  = 300.0f;
  const float dialog_height = 100.0f;
  const float dialog_x      = (Game::width - dialog_width) * 0.5f;
  const float dialog_y      = Game::height - dialog_height - 10.0f;
  DrawRectangle(dialog_x, dialog_y, dialog_width, dialog_height, Color{ 16, 16, 32, 200 });
  DrawRectangleLinesEx(Rectangle{ dialog_x, dialog_y, dialog_width, dialog_height }, 1, Color{ 255, 224, 255, 255 });

  // name
  {
    DrawTextEx(font,
               dialog->actor_name.c_str(),
               Vector2{ dialog_x + 10.0f + 1.0f, dialog_y + 10.0f + 1.0f },
               font_size,
               2.0f,
               DARKBLUE);
    DrawTextEx(
      font, dialog->actor_name.c_str(), Vector2{ dialog_x + 10.0f, dialog_y + 10.0f }, font_size, 2.0f, RAYWHITE);
    auto name_size = MeasureTextEx(font, dialog->actor_name.c_str(), font_size, 2.0f);
    DrawLineEx(Vector2{ dialog_x + 10.0f, dialog_y + 10.0f + name_size.y },
               Vector2{ dialog_x + 10.0f + name_size.x, dialog_y + 10.0f + name_size.y },
               1.0f,
               RAYWHITE);
  }

  // text
  {
    const float line_spacing = font_size - 2.0f;
    SetTextLineSpacing(line_spacing);
    const std::string &t = dialog->text;

    if (t.find('$') != std::string::npos)
    {
      const float text_x = dialog_x + 10.0f;
      const float text_y = dialog_y + 10.0f + font_size + 5.0f;
      float x            = text_x;
      float y            = text_y;
      Color color        = WHITE;

      for (size_t i = 0; i < t.size(); i++)
      {
        const char c = t[i];
        if (c == '$' && i + 1 < t.size())
        {
          if (t[i + 1] == '1')
          {
            color = CRYSTAL_COLOR;
            i++;
            continue;
          }
          else if (t[i + 1] == '0')
          {
            color = WHITE;
            i++;
            continue;
          }
        }
        if (c == '\n')
        {
          x = text_x;
          y += line_spacing;
          continue;
        }

        const auto text = TextFormat("%c", c);
        DrawTextEx(dialog_font, text, Vector2{ x, y }, font_size, 0.0f, color);
        x += MeasureTextEx(dialog_font, text, font_size, 0.0f).x + 1.0f;
        if (x > dialog_x + dialog_width - 10.0f)
        {
          x = text_x;
          y += line_spacing;
        }
      }
    }
    else
    {
      DrawTextEx(dialog_font,
                 t.c_str(),
                 Vector2{ dialog_x + 10.0f, dialog_y + 10.0f + font_size + 5.0f },
                 font_size,
                 1.0f,
                 WHITE);
    }
  }
  const auto text_size = MeasureTextEx(dialog_font, dialog->text.c_str(), font_size, 1.0f);

  // responses
  const Color selected_color = selection_color();
  const float response_x     = dialog_x + 20.0f + 5.0f;
  const float response_y     = dialog_y + 10.0f + text_size.y + font_size * 2.0f;
  for (size_t i = 0; i < dialog->responses.size(); i++)
  {
    const DialogResponse &response = dialog->responses[i];
    DrawTextEx(dialog_font,
               response.text.c_str(),
               Vector2{ response_x, response_y + (font_size + 5.0f) * i },
               font_size,
               1.0f,
               WHITE);

    if (selected_index.has_value() && selected_index.value() == i)
    {
      DrawTextEx(dialog_font,
                 response.text.c_str(),
                 Vector2{ response_x, response_y + (font_size + 5.0f) * i },
                 font_size,
                 1.0f,
                 selected_color);

      const float triangle_size = 8.0f;
      const float triangle_x    = dialog_x + 10.0f;
      const float triangle_y    = response_y + font_size * 0.5f + (font_size + 5.0f) * i;
      DrawTriangle(Vector2{ triangle_x, triangle_y - triangle_size * 0.5f },
                   Vector2{ triangle_x, triangle_y + triangle_size * 0.5f },
                   Vector2{ triangle_x + triangle_size, triangle_y },
                   selected_color);
    }
  }
}

bool GUI::is_active() const
{
  return dialog.has_value();
}

void GUI::draw_shop_items(const std::vector<ShopItem> &items) const noexcept
{
  std::unordered_map<ShopItem::AvailabilityReason, std::string> buy_text_map{
    { ShopItem::AvailabilityReason::Available, "Buy" },
    { ShopItem::AvailabilityReason::AlreadyOwned, "Owned" },
    { ShopItem::AvailabilityReason::NotEnoughMoney, "Cannot afford" },
    { ShopItem::AvailabilityReason::NotAvailable, "Sold out" },
  };

  const std::string header = "Upgrade Ship";

  draw_selectable_items(header, items, buy_text_map);
}

void GUI::draw_ship_items(const std::vector<ShopItem> &items) const noexcept
{
  std::unordered_map<ShopItem::AvailabilityReason, std::string> buy_text_map{
    { ShopItem::AvailabilityReason::Available, "Equip" },
    { ShopItem::AvailabilityReason::AlreadyOwned, "Equipped" },
    { ShopItem::AvailabilityReason::NotEnoughMoney, "" },
    { ShopItem::AvailabilityReason::NotAvailable, "" },
  };

  const std::string header = "Modify Ship";

  draw_selectable_items(header, items, buy_text_map);
}

void GUI::draw_selectable_items(
  const std::string &header,
  const std::vector<ShopItem> &items,
  const std::unordered_map<ShopItem::AvailabilityReason, std::string> &buy_text_map) const noexcept
{
  const float MARGIN         = 10.0f;
  const Color selected_color = selection_color();

  // dialog background
  const float dialog_width  = 300.0f;
  const float dialog_height = MARGIN * 5.0f + (font_size * 2.0f + MARGIN * 2.0f) * static_cast<float>(items.size());
  const float dialog_x      = std::roundf((Game::width - dialog_width) * 0.5f);
  const float dialog_y      = std::roundf((Game::height - dialog_height) * 0.5f - 20.0f);
  DrawRectangle(dialog_x, dialog_y, dialog_width, dialog_height, Color{ 16, 16, 32, 200 });
  DrawRectangleLinesEx(Rectangle{ dialog_x, dialog_y, dialog_width, dialog_height }, 1, Color{ 255, 224, 255, 255 });

  // header
  Vector2 header_text_size{ 0.0f, -MARGIN };
  if (!header.empty())
  {
    const char *header_text = header.c_str();
    header_text_size        = MeasureTextEx(font, header_text, font_size, 1.0f);
    DrawTextEx(font,
               header_text,
               Vector2{ dialog_x + std::roundf(dialog_width / 2.0f - header_text_size.x / 2.0f), dialog_y + MARGIN },
               font_size,
               1.0f,
               WHITE);

    DrawLineV(Vector2{ dialog_x + MARGIN, dialog_y + MARGIN + header_text_size.y + MARGIN },
              Vector2{ dialog_x + dialog_width - MARGIN, dialog_y + MARGIN + header_text_size.y + MARGIN },
              WHITE);
  }

  // items
  float drawn_count = 0.0f;
  for (size_t i = 0; i < items.size(); i++)
  {
    const bool is_selected = selected_index.has_value() && selected_index.value() == i;

    const auto &item = items[i];

    Color color = WHITE;
    if (is_selected)
      color = selected_color;

    const auto availability = item.is_available();

#if defined(DEBUG_GUI_ITEM_AVAILABILITY)
    DrawText(TextFormat("%s", magic_enum::enum_name(availability).data()),
             dialog_x + dialog_width - 100.0f,
             dialog_y + (MARGIN + 10.0f) * i,
             10.0f,
             color);
#endif

    if (!buy_text_map.contains(availability) || buy_text_map.at(availability).empty())
      continue;

    if (availability != ShopItem::AvailabilityReason::Available)
      color = DARKGRAY;

    // item box
    const float item_box_w = dialog_width - MARGIN * 2.0f;
    const float item_box_h = font_size + MARGIN * 2.0f;
    const float item_box_x = dialog_x + MARGIN;
    const float item_box_y =
      dialog_y + MARGIN * 2.0f + header_text_size.y + MARGIN + (item_box_h + MARGIN) * drawn_count;

    if (is_selected)
      DrawRectangle(item_box_x, item_box_y, item_box_w, item_box_h, Color{ 16, 16, 32, 200 });

    // item icon
    const float item_icon_size = item_box_h;
    const float item_icon_x    = item_box_x;
    const float item_icon_y    = item_box_y;
    DrawRectangle(item_icon_x, item_icon_y, item_icon_size, item_icon_size, Color{ 16, 16, 32, 200 });

    // item name
    const Vector2 item_name_size = MeasureTextEx(font, item.name.c_str(), font_size, 1.0f);
    DrawTextEx(font,
               item.name.c_str(),
               Vector2{ item_icon_x + item_icon_size + MARGIN * 0.5f + 1.0f,
                        item_icon_y + item_icon_size * 0.5f - item_name_size.y + 1.0f },
               font_size,
               1.0f,
               BLACK);
    DrawTextEx(
      font,
      item.name.c_str(),
      Vector2{ item_icon_x + item_icon_size + MARGIN * 0.5f, item_icon_y + item_icon_size * 0.5f - item_name_size.y },
      font_size,
      1.0f,
      color);

    // item description
    DrawTextEx(dialog_font,
               item.description.c_str(),
               Vector2{ item_icon_x + item_icon_size + MARGIN * 0.5f, item_icon_y + item_icon_size * 0.5f },
               font_size,
               0.0f,
               color);

    // buy button
    const char *buy_text          = buy_text_map.contains(availability) ? buy_text_map.at(availability).c_str() : "";
    const char *price_text        = TextFormat("%4zu", item.price);
    const Vector2 buy_text_size   = MeasureTextEx(font, buy_text, font_size, 0.0f);
    const Vector2 price_text_size = MeasureTextEx(font, price_text, font_size, 1.0f);
    const float buy_button_w      = std::max(80.0f, buy_text_size.x + price_text_size.x + MARGIN * 3.0f);
    const float buy_button_h      = 20.0f;
    const float buy_button_x      = std::roundf(dialog_x + dialog_width - buy_button_w - MARGIN * 1.5f);
    const float buy_button_y      = std::roundf(item_box_y + item_box_h * 0.5f - buy_button_h * 0.5f);
    DrawRectangle(buy_button_x, buy_button_y, buy_button_w, buy_button_h, Color{ 16, 16, 32, 200 });
    DrawRectangleLinesEx(
      Rectangle{ buy_button_x, buy_button_y, buy_button_w, buy_button_h }, is_selected ? 2.0f : 1.0f, color);

    const float buy_text_x = std::roundf(buy_button_x + MARGIN);
    const float buy_text_y = std::roundf(buy_button_y + buy_button_h * 0.5f - buy_text_size.y * 0.5f);
    DrawTextEx(font, buy_text, Vector2{ buy_text_x, buy_text_y }, font_size, 0.0f, color);

    if (item.price > 0)
    {
      const float price_text_x =
        std::roundf(buy_button_x + buy_button_w - price_text_size.x - MARGIN - ui_crystal->get_width() * 0.4f);
      const float price_text_y = std::roundf(buy_button_y + buy_button_h * 0.5f - price_text_size.y * 0.5f);

      DrawTextEx(font, price_text, Vector2{ price_text_x, price_text_y }, font_size, 1.0f, color);
      ui_crystal->set_frame(0);
      ui_crystal->scale = Vector2{ 0.4f, 0.4f };
      ui_crystal->set_centered();
      ui_crystal->position.x = price_text_x + price_text_size.x + 4.0f;
      ui_crystal->position.y = price_text_y + price_text_size.y * 0.5f;
      ui_crystal->draw();
    }

    if (is_selected)
      DrawRectangleLinesEx(Rectangle{ item_box_x, item_box_y, item_box_w, item_box_h }, 1, color);

    drawn_count += 1.0f;
  }

  // exit button
  const bool selected_exit  = selected_index == items.size();
  const float exit_button_w = 50.0f;
  const float exit_button_h = 20.0f;
  Color color               = WHITE;
  if (selected_exit)
    color = selected_color;
  const float exit_button_x = dialog_x + dialog_width - exit_button_w;
  const float exit_button_y = dialog_y + dialog_height + MARGIN;
  DrawRectangle(exit_button_x, exit_button_y, exit_button_w, exit_button_h, Color{ 16, 16, 32, 200 });
  DrawRectangleLinesEx(
    Rectangle{ exit_button_x, exit_button_y, exit_button_w, exit_button_h }, selected_exit ? 2.0f : 1.0f, color);
  const Vector2 exit_text_size = MeasureTextEx(font, "Exit", font_size, 1.0f);
  DrawTextEx(
    font,
    "Exit",
    Vector2{ exit_button_x + exit_button_w * 0.5f - std::roundf(exit_text_size.x * 0.5f), exit_button_y + 5.0f },
    font_size,
    1.0f,
    color);
}

void GUI::handle_selecting_index(std::optional<size_t> &index, size_t max_index) const noexcept
{
  if (!index)
    return;

  if (IsKeyPressed(KEY_DOWN))
  {
    index.value()++;
    if (index >= max_index)
      index = 0;
  }

  if (IsKeyPressed(KEY_UP))
  {
    if (index == 0)
      index = max_index - 1;
    else
      index.value()--;
  }
}

void GUI::handle_accepting_index(std::optional<size_t> &index, std::function<void(size_t)> func) const noexcept
{
  if (!index)
    return;

  if (IsKeyPressed(KEY_SPACE) || IsKeyPressedRepeat(KEY_SPACE))
    func(index.value());
}
