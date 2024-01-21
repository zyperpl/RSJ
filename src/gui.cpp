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
  mono_font   = LoadFontEx("resources/Kenney Mini Square Mono.ttf", 10, nullptr, 0);

  ui_crystal = std::make_unique<Sprite>("resources/ore.aseprite");

  name_icon_map.emplace("Orbital Perimeter", Sprite{ "resources/mission_1.aseprite" });
  name_icon_map.emplace("Nearfield Zone", Sprite{ "resources/mission_2.aseprite" });
  name_icon_map.emplace("Inner asteroid belt", Sprite{ "resources/mission_3.aseprite" });
  name_icon_map.emplace("Close Quarters Space", Sprite{ "resources/mission_4.aseprite" });
  name_icon_map.emplace("Outer asteroid belt", Sprite{ "resources/mission_5.aseprite" });
  name_icon_map.emplace("Trans-Neptunian Region", Sprite{ "resources/mission_6.aseprite" });
  name_icon_map.emplace("Interstellar Space", Sprite{ "resources/mission_7.aseprite" });
  name_icon_map.emplace("Galactic Core", Sprite{ "resources/mission_8.aseprite" });
  name_icon_map.emplace("Intergalactic Space", Sprite{ "resources/mission_9.aseprite" });

  name_icon_map.emplace("Normal Gun", Sprite{ "resources/weapon_1.aseprite" });
  name_icon_map.emplace("Fast Gun", Sprite{ "resources/weapon_2.aseprite" });
  name_icon_map.emplace("Auto Gun", Sprite{ "resources/weapon_3.aseprite" });
  name_icon_map.emplace("Homing Gun", Sprite{ "resources/weapon_4.aseprite" });

  name_icon_map.emplace("Mission Select", Sprite{ "resources/mission_select.aseprite" });
  name_icon_map.emplace("Change Weapons", Sprite{ "resources/change_weapons.aseprite" });
}

GUI::~GUI()
{
  UnloadFont(font);
  UnloadFont(dialog_font);
}

void GUI::draw() const noexcept
{
  const Color special_color = selection_color();
  const Game &game          = Game::get();

  Vector2 text_position{ 10.0f, 10.0f };
  if (CONFIG(show_fps))
  {
    DrawTextEx(font, TextFormat("FPS: %i", GetFPS()), text_position, font_size, 1.0f, WHITE);
    text_position.y += font_size + 5.0f;
  }

  const char *lives_text = "Lives: ";
  DrawTextEx(font, lives_text, Vector2Add(text_position, Vector2{ 0.0f, 1.0f }), font_size, 1.0f, BLACK);
  DrawTextEx(font, lives_text, text_position, font_size, 1.0f, WHITE);
  Vector2 text_size = MeasureTextEx(font, lives_text, font_size, 1.0f);
  float x           = text_position.x + text_size.x + 5.0f;
  float y           = text_position.y + text_size.y * 0.5f;
  for (int i = 0; i < game.player->max_lives; i++)
  {
    if (i <= game.player->lives - 1)
    {
      DrawCircleV(Vector2{ x + i * 11.0f, y }, 5.0f, BEIGE);
      DrawCircleLinesV(Vector2{ x + i * 11.0f, y }, 5.0f, ColorBrightness(BEIGE, -0.3f));
    }
    else
      DrawCircleLinesV(Vector2{ x + i * 11.0f, y }, 5.0f, ColorAlpha(RED, 0.5f));
  }

  text_position.y += font_size + 5.0f;
  static uint64_t draw_score = 0;
  uint64_t score_step        = std::max<uint64_t>(1, (game.score - draw_score) / 20);
  if (draw_score < game.score)
    draw_score += score_step;
  if (draw_score > game.score)
    draw_score = game.score;
  DrawTextEx(font,
             TextFormat("Score: %i", draw_score),
             Vector2Add(text_position, Vector2{ 0.0f, 1.0f }),
             font_size,
             1.0f,
             BLACK);
  DrawTextEx(font, TextFormat("Score: %i", draw_score), text_position, font_size, 1.0f, WHITE);

  text_position.y += font_size + 5.0f;
  const char *crystals_text = TextFormat("Crystals: %i", game.crystals);
  DrawTextEx(font, crystals_text, Vector2Add(text_position, Vector2{ 0.0f, 1.0f }), font_size, 1.0f, BLACK);
  DrawTextEx(font, crystals_text, text_position, font_size, 1.0f, WHITE);
  text_size = MeasureTextEx(font, crystals_text, font_size, 1.0f);
  assert(ui_crystal);
  ui_crystal->set_frame(0);
  ui_crystal->scale = Vector2{ 0.4f, 0.4f };
  ui_crystal->set_centered();
  ui_crystal->position.x = text_position.x + text_size.x + 5.0f;
  ui_crystal->position.y = text_position.y + text_size.y * 0.5f;
  ui_crystal->draw();

  text_position.y += font_size + 5.0f;

  if (!game.artifacts.empty())
  {
    const char *artifacts_text = TextFormat("Artifacts: %i", game.artifacts.size());
    DrawTextEx(font, artifacts_text, Vector2Add(text_position, Vector2{ 0.0f, 1.0f }), font_size, 1.0f, BLACK);
    DrawTextEx(font, artifacts_text, text_position, font_size, 1.0f, WHITE);
  }

  // draw quests
  {
    const float quest_right_margin = 10.0f;
    float quest_y                  = 10.0f;
    for (const auto &[quest_name, quest] : game.quests)
    {
#if !defined(DEBUG_QUESTS)
      if (!quest.is_accepted() || quest.is_reported())
        continue;
#else
      DrawText(TextFormat("%i/%i", quest.progress(), quest.max_progress()),
               Game::width - quest_right_margin - 50.0f,
               quest_y + 5.0f,
               font_size,
               WHITE);
#endif

      const auto &quest_text =
        TextFormat("%s: %i/%i", quest.description.c_str(), quest.progress(), quest.max_progress());
      const float quest_x = Game::width - MeasureTextEx(font, quest_text, font_size, 1.0f).x - quest_right_margin;
      const Color color   = quest.is_completed() ? LIME : WHITE;
      DrawTextEx(font, quest_text, Vector2{ quest_x, quest_y + 1.0f }, font_size, 1.0f, BLACK);
      DrawTextEx(font, quest_text, Vector2{ quest_x, quest_y }, font_size, 1.0f, color);
      quest_y += font_size + 5.0f;
    }
  }

  if (!messages.empty())
  {
    float total_y = Game::height * 0.1f;
    for (const auto &message : messages)
    {
      if (!message.timer.is_done())
      {
        const float letter_spacing = 0.0f;

        const Vector2 text_size = MeasureTextEx(font, message.text.c_str(), font_size, letter_spacing);
        const float message_x   = std::roundf(Game::width * 0.5f - text_size.x * 0.5f);
        const float r           = message.timer.get_ratio() * PI;
        const float y_offset    = std::clamp(std::sin(r), 0.0f, 0.8f) * 1.25f;
        const float message_y   = std::roundf(-text_size.y - 8.0f + y_offset * total_y);

        const float margin_w = 6.0f;
        const float margin_h = 4.0f;
        const Rectangle bg_rectangle{
          message_x - margin_w, message_y - margin_h, text_size.x + margin_w * 2.0f, text_size.y + margin_h * 2.0f
        };
        DrawRectangleRounded(bg_rectangle, 0.5f, 12, Color{ 16, 16, 32, 220 });
        DrawRectangleRoundedLines(bg_rectangle, 0.5f, 12, 2.0f, Color{ 16, 16, special_color.g, 250 });

        for (int y = -1; y <= 1; y++)
        {
          for (int x = -1; x <= 1; x++)
          {
            DrawTextEx(
              font, message.text.c_str(), Vector2{ message_x + x, message_y + y }, font_size, letter_spacing, BLACK);
          }
        }
        DrawTextEx(
          font, message.text.c_str(), Vector2{ message_x, message_y }, font_size, letter_spacing, special_color);

        total_y += text_size.y * 2.0f;
      }
    }
  }

  if (game.player && !dialog.has_value() && !GAME.freeze_entities && game.player->can_interact())
  {
    if (auto entity = game.player->get_interactable(); entity)
    {
      const std::string &interact_text = entity->get_interact_text();
      if (entity->is_interactable() && !interact_text.empty())
      {
        const std::string text_a   = "Press ";
        const std::string text     = text_a + "SPACE to " + interact_text;
        const float letter_spacing = 0.0f;
        const Color color          = WHITE;
        const float margin_w       = 4.0f;
        const float margin_h       = 2.0f;

        const Vector2 pos =
          Vector2Subtract(entity->get_sprite().position, Vector2Subtract(GAME.camera.target, GAME.camera.offset));
        const Vector2 text_a_size = MeasureTextEx(font, text_a.c_str(), font_size, letter_spacing);
        const Vector2 text_size   = MeasureTextEx(font, text.c_str(), font_size, letter_spacing);
        float message_x           = std::roundf(pos.x - text_size.x * 0.5f);
        if (message_x < margin_w)
          message_x = margin_w;
        else if (message_x + text_size.x > Game::width - margin_w)
          message_x = Game::width - text_size.x - margin_w;

        float message_y = std::roundf(pos.y - entity->get_sprite().get_height() * 0.5f - text_size.y - 8.0f);
        if (message_y < margin_h)
          message_y = margin_h;
        else if (message_y + text_size.y > Game::height - margin_h)
          message_y = Game::height - text_size.y - margin_h;

        const Rectangle bg_rectangle{
          message_x - margin_w, message_y - margin_h, text_size.x + margin_w * 2.0f, text_size.y + margin_h * 2.0f
        };
        DrawRectangleRounded(bg_rectangle, 0.5f, 12, Color{ 16, 16, 32, 220 });

        for (int y = -1; y <= 1; y++)
        {
          for (int x = -1; x <= 1; x++)
          {
            DrawTextEx(font, text.c_str(), Vector2{ message_x + x, message_y + y }, font_size, letter_spacing, BLACK);
          }
        }
        DrawTextEx(font, text.c_str(), Vector2{ message_x, message_y }, font_size, letter_spacing, color);

        const float special_x = message_x + text_a_size.x;
        const float special_y = message_y;
        DrawTextEx(font, "SPACE", Vector2{ special_x, special_y }, font_size, letter_spacing, special_color);
      }
    }
  }

  if (game.state == GameState::PLAYING_ASTEROIDS)
  {
    if (game.survive_time > 0.0f)
    {
      Color color                = WHITE;
      const float survive_time   = game.survive_time;
      const float seconds        = std::floor(std::fmod(survive_time, 60.0f));
      const float milliseconds   = std::floor(std::fmod(survive_time, 1.0f) * 100.0f);
      const std::string text     = TextFormat("Survive: %02.0f:%02.0f", seconds, milliseconds);
      const float letter_spacing = 0.0f;
      const Vector2 text_size    = MeasureTextEx(mono_font, text.c_str(), font_size, letter_spacing);
      if (seconds < 10.0f)
        color = special_color;

      DrawTextEx(mono_font,
                 text.c_str(),
                 Vector2{ Game::width * 0.5f - text_size.x * 0.5f, font_size + 10.0f },
                 font_size,
                 letter_spacing,
                 color);
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
    const float letter_spacing = 0.0f;
    const float line_spacing   = font_size - 2.0f;
    SetTextLineSpacing(line_spacing);
    const std::string &t = dialog->text;

    const std::unordered_map<char, Color> font_colors{
      { '0', WHITE },
      { '1', CRYSTAL_COLOR },
      { '2', GOLD },
    };

    const bool advanced_text = t.find('$') != std::string::npos || t.find('&') != std::string::npos;
    if (advanced_text)
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
          const char &next = t[i + 1];
          if (font_colors.contains(next))
          {
            color = font_colors.at(next);
            i++;
            continue;
          }
        }
        else if (c == '&' && i + 1 < t.size())
        {
          const char &next = t[i + 1];
          if (next == 'U')
          {
            const float triangle_size = 8.0f;
            const float triangle_x    = x;
            const float triangle_y    = y + triangle_size * 0.1f;
            DrawTriangle(Vector2{ triangle_x + triangle_size, triangle_y + triangle_size },
                         Vector2{ triangle_x + triangle_size * 0.5f, triangle_y },
                         Vector2{ triangle_x, triangle_y + triangle_size },
                         color);

            x += triangle_size + letter_spacing + 1.0f;
            i++;
            continue;
          }
        }
        else if (c == '\n')
        {
          x = text_x;
          y += line_spacing;
          continue;
        }

        const auto text = TextFormat("%c", c);
        DrawTextEx(dialog_font, text, Vector2{ x, y }, font_size, 0.0f, color);
        x += MeasureTextEx(dialog_font, text, font_size, 0.0f).x + letter_spacing;
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
                 letter_spacing,
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

void GUI::draw_ship_control(const std::vector<ShopItem> &items) const noexcept
{
  draw_selectable_items("", items, {});
}

void GUI::draw_selectable_items(
  const std::string &header,
  const std::vector<ShopItem> &items,
  const std::unordered_map<ShopItem::AvailabilityReason, std::string> &buy_text_map) const noexcept
{
  const float MARGIN{ 10.0f };
  const Color selected_color = selection_color();
  const Color scrollbar_color{ 255, 255, 255, 180 };

  // dialog background
  const float dialog_width = 300.0f;
  float dialog_height      = MARGIN * 5.0f + (font_size * 2.0f + MARGIN * 2.0f) * static_cast<float>(items.size());
  if (dialog_height > GAME.height * 0.8f)
    dialog_height = GAME.height * 0.8f;
  const float dialog_x = std::roundf((Game::width - dialog_width) * 0.5f);
  const float dialog_y = std::roundf((Game::height - dialog_height) * 0.5f - 20.0f);
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

  BeginScissorMode(static_cast<int>(dialog_x),
                   static_cast<int>(dialog_y + header_text_size.y + MARGIN * 2.0f),
                   static_cast<int>(dialog_width),
                   static_cast<int>(dialog_height - header_text_size.y - MARGIN * 3.0f + 2.0f));

  // items
  float drawn_count                = 0.0f;
  static float selected_index_draw = 0.0f;
  const float selected_index_value = selected_index.has_value() ? selected_index.value() : 0.0f;
  //selected_index_draw              = Lerp(selected_index_draw, selected_index_value, 0.1f);
  // Lerp with time
  static double previous_time = GetTime();
  const double current_time   = GetTime();
  const float dt              = static_cast<float>(current_time - previous_time);
  previous_time               = current_time;
  selected_index_draw = Lerp(selected_index_draw, selected_index_value, 0.1f * dt * 60.0f);


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

    // skip unavailable items if state explicitly defined as empty
    if (!buy_text_map.empty())
    {
      if (!buy_text_map.contains(availability) || buy_text_map.at(availability).empty())
        continue;
    }

    if (availability != ShopItem::AvailabilityReason::Available)
      color = DARKGRAY;

    // item box
    const float item_box_w = dialog_width - MARGIN * 2.0f;
    const float item_box_h = font_size + MARGIN * 2.0f;

    float scroll_offset = 0.0f;

    const float entries_count = items.size() - 1.0f;
    if (entries_count * item_box_h > dialog_height - header_text_size.y - MARGIN * 3.0f)
    {
      scroll_offset          = std::roundf((selected_index_draw - 1.0f) * item_box_h);
      const float max_scroll = std::min((entries_count - 2.0f) * item_box_h - MARGIN, dialog_height);
      scroll_offset          = std::clamp(scroll_offset, 0.0f, max_scroll);

      // scroll bar
      const float scroll_ratio  = scroll_offset / (font_size + MARGIN * 2.0f) / (entries_count - 1.5f);
      const float scroll_height = dialog_height - header_text_size.y - MARGIN * 3.0f;
      const float scroll_y      = dialog_y + header_text_size.y + MARGIN * 2.0f + scroll_height * scroll_ratio;
      DrawRectangle(dialog_x + dialog_width - MARGIN,
                    scroll_y,
                    MARGIN * 0.25f,
                    scroll_height / (entries_count - 1.0f),
                    scrollbar_color);

      // items upper overflow line
      if (scroll_offset > 0.0f)
      {
        DrawLineV(Vector2{ dialog_x + MARGIN * 2.0f, dialog_y + MARGIN + header_text_size.y + MARGIN },
                  Vector2{ dialog_x + dialog_width - MARGIN * 2.0f, dialog_y + MARGIN + header_text_size.y + MARGIN },
                  scrollbar_color);
      }
    }

    const float item_box_x = dialog_x + MARGIN;
    const float item_box_y =
      dialog_y + MARGIN * 2.0f + header_text_size.y + MARGIN + (item_box_h + MARGIN) * (drawn_count)-scroll_offset;

    if (is_selected)
      DrawRectangle(item_box_x, item_box_y, item_box_w, item_box_h, Color{ 16, 16, 32, 200 });

    // item icon background
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
    if (!item.description.empty())
    {
      DrawTextEx(dialog_font,
                 item.description.c_str(),
                 Vector2{ item_icon_x + item_icon_size + MARGIN * 0.5f, item_icon_y + item_icon_size * 0.5f },
                 font_size,
                 0.0f,
                 color);
    }

    // buy button
    if (buy_text_map.contains(availability))
    {
      const char *buy_text          = buy_text_map.at(availability).c_str();
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
    }

    if (is_selected)
      DrawRectangleLinesEx(Rectangle{ item_box_x, item_box_y, item_box_w, item_box_h }, 1, color);

    // item icon
    if (name_icon_map.contains(item.name))
    {
      auto &icon      = name_icon_map.at(item.name);
      icon.position.x = item_icon_x;
      icon.position.y = item_icon_y;
      icon.draw();
    }

    // items lower overflow line
    if (item_box_y + item_box_h >= dialog_y + dialog_height)
    {
      DrawLineV(Vector2{ dialog_x + MARGIN * 2.0f, dialog_y + dialog_height - MARGIN + 1.0f },
                Vector2{ dialog_x + dialog_width - MARGIN * 2.0f, dialog_y + dialog_height - MARGIN + 1.0f },
                scrollbar_color);
    }

    drawn_count += 1.0f;
  }

  EndScissorMode();

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

  if (max_index <= 1)
    return;

  const Game &game = Game::get();

  if (game.input.menu_down_pressed())
  {
    sound_select.play();
    index.value()++;
    if (index >= max_index)
      index = 0;
  }

  if (game.input.menu_up_pressed())
  {
    sound_select.play();
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

  const Game &game = Game::get();
  if (game.input.menu_action_pressed())
  {
    sound_accept.play();
    func(index.value());
  }
}

void GUI::show_message(const std::string &new_message)
{
  Message message{ new_message };
  message.timer.start();
  messages.push_back(message);
}
