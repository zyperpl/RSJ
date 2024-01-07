#include "game.hpp"

#include <cassert>

#include <raylib.h>
#include <raymath.h>

#include "magic_enum/magic_enum.hpp"

#include "asteroid.hpp"
#include "bullet.hpp"
#include "interactable.hpp"
#include "object_circular_buffer.hpp"
#include "particle.hpp"
#include "pickable.hpp"
#include "player_character.hpp"
#include "player_ship.hpp"
#include "room.hpp"
#include "utils.hpp"

static constexpr const float TRANSITION_SPEED{ 1.25f };

void Game::schedule_action_change_level(const Level &level, size_t mission, const Interactable *obj) noexcept
{
  TraceLog(LOG_INFO, "Changing level to %i (mission: %i)", static_cast<int>(level), mission);

  // player animation
  {
    Action action;
    action.on_update = [this, target_rect = obj->get_sprite().get_destination_rect()](Action &action)
    {
      bool reached_interactable{ false };
      const Vector2 target_position{ target_rect.x, target_rect.y };
      if (PlayerShip *player_ship = dynamic_cast<PlayerShip *>(player.get()); player_ship)
      {
        player_ship->position = Vector2Lerp(player_ship->position, target_position, 0.05f);

        if (player_ship->sprite.scale.x > 0.1f)
          player_ship->sprite.scale = Vector2Scale(player_ship->sprite.scale, 0.9f);

        reached_interactable =
          Vector2Distance(player_ship->sprite.position, target_position) < 4.0f || player_ship->sprite.scale.x < 0.1f;
      }
      if (PlayerCharacter *player_character = dynamic_cast<PlayerCharacter *>(player.get()); player_character)
      {
        const Vector2 pos{ player_character->position.x, player_character->position.y + 8.0f };
        const float walk_speed = 0.5f;
        if (pos.x < target_position.x)
          player_character->velocity.x = walk_speed;
        else if (pos.x > target_position.x)
          player_character->velocity.x = -walk_speed;
        else
          player_character->velocity.x = 0.0f;

        if (pos.y < target_position.y)
          player_character->velocity.y = walk_speed;
        else if (pos.y > target_position.y)
          player_character->velocity.y = -walk_speed;
        else
          player_character->velocity.y = 0.0f;

        player_character->position = Vector2Add(player_character->position, player_character->velocity);
        player_character->animate();

        if (Vector2Distance(pos, target_position) < 8.0f)
          reached_interactable = true;
      }

      freeze_entities = true;

      if (reached_interactable)
        action.is_done = true;
    };

    actions.push(std::move(action));
  }

  // fade-out transition
  {
    Action action;
    action.on_update = [](Action &action)
    {
      action.data = std::get<float>(action.data) + DELTA_TIME * TRANSITION_SPEED;

      if (std::get<float>(action.data) >= 1.0f)
        action.is_done = true;
    };

    action.on_draw = [](const Action &action)
    {
      const float &data = std::get<float>(action.data);
      const float size  = std::max(width, height) * 0.5f * data;
      DrawPoly(Vector2{ width * 0.5f, height * 0.5f }, 16, size, data * 0.1f, BLACK);
    };

    action.on_done = [this, level, mission](Action &)
    {
      switch (level)
      {
        case Level::None:
          assert(false);
          break;
        case Level::Asteroids:
          set_mission(mission);
          set_state(GameState::PLAYING_ASTEROIDS);
          break;
        case Level::Station:
          set_state(GameState::PLAYING_STATION);
          set_room(Room::Type::DockingBay);
          break;
      }
    };

    action.data = 0.0f;
    actions.push(std::move(action));
  }

  // fade-in transition
  {
    Action action;
    action.on_update = [](Action &action)
    {
      action.data = std::get<float>(action.data) + DELTA_TIME * TRANSITION_SPEED;

      if (std::get<float>(action.data) >= 1.0f)
        action.is_done = true;
    };

    action.on_draw = [](const Action &action)
    {
      const float &data = std::get<float>(action.data);
      DrawRing(Vector2{ width * 0.5f, height * 0.5f }, width * data, width, 0.0f, 360.0f, 16, BLACK);
    };

    action.on_done = [this](Action &) { freeze_entities = false; };

    action.data = 0.0f;
    actions.push(std::move(action));
  }
}

void Game::schedule_action_conversation(DialogEntity &entity) noexcept
{
  Action action;

  action.on_start = [this, &entity](Action &action)
  {
    TraceLog(LOG_INFO, "Playing dialog %p(%s)", (void *)(&entity), entity.get_dialog_id().c_str());
    freeze_entities = true;

    gui->set_dialog(entity.dialog());
    if (gui->dialog->actor_name == Dialog::END_DIALOG_ID)
    {
      TraceLog(LOG_INFO, "Dialog ended");
      action.is_done = true;
      return;
    }

    if (!gui->dialog->responses.empty())
      gui->selected_index = 0;
  };

  action.on_update = [this](Action &action)
  {
    if (!gui->dialog->responses.empty() && !gui->selected_index.has_value())
      gui->selected_index.value() = 0;

    if (gui->selected_index.has_value())
    {
      gui->handle_selecting_index(gui->selected_index, gui->dialog->responses.size());
      gui->handle_accepting_index(gui->selected_index,
                                  [&](size_t index)
                                  {
                                    TraceLog(LOG_TRACE, "Selected dialog response: %zu", index);
                                    const auto &response       = gui->dialog->responses[index];
                                    const auto &next_dialog_id = response.next_dialog_id;

                                    if (response.func)
                                      response.func();

                                    if (next_dialog_id.starts_with('_'))
                                    {
                                      if (next_dialog_id == "_end")
                                        TraceLog(LOG_INFO, "Dialog ended");
                                      else if (next_dialog_id == "_shop")
                                        schedule_action_shop(nullptr);
                                      else if (next_dialog_id == "_ship")
                                        schedule_action_modify_ship(nullptr);
                                      else
                                        TraceLog(LOG_WARNING, "Unknown dialog id: %s", next_dialog_id.c_str());
                                    }
                                    else
                                    {
                                      action.data = DialogId{ next_dialog_id };
                                    }

                                    action.is_done = true;
                                  });
    }
  };

  action.on_draw = [this](const Action &action)
  {
    if (action.has_started)
      gui->draw_dialog();
  };

  action.on_done = [this, &entity](Action &action)
  {
    gui->reset_dialog();

    if (std::holds_alternative<DialogId>(action.data))
    {
      const auto &next_dialog_id = std::get<DialogId>(action.data);
      entity.set_dialog_id(next_dialog_id);
      schedule_action_conversation(entity);
    }
    freeze_entities = false;
  };

  actions.push(std::move(action));
}

void Game::schedule_action_shop(const Interactable *interactable) noexcept
{
  TraceLog(LOG_INFO, "Playing shop %p", (void *)interactable);

  std::shared_ptr<std::vector<ShopItem>> shop_items = std::make_shared<std::vector<ShopItem>>();

  Action action;
  action.on_start = [this, shop_items](Action &)
  {
    freeze_entities = true;

    shop_items->push_back(ShopItem{ .name        = "Fast Gun",
                                    .description = "Shoots faster",
                                    .price       = 10,
                                    .on_accept =
                                      [this]
                                    {
                                      crystals -= 10;
                                      guns[GunType::Fast] = true;
                                    },
                                    .on_has_item     = [this](const ShopItem &) { return guns[GunType::Fast]; },
                                    .on_is_available = [](const ShopItem &) { return true; } });
    shop_items->push_back(ShopItem{ .name        = "Auto Gun",
                                    .description = "Auto-aims at enemies",
                                    .price       = 60,
                                    .on_accept =
                                      [this]
                                    {
                                      crystals -= 60;
                                      guns[GunType::Assisted] = true;
                                    },
                                    .on_has_item     = [this](const ShopItem &) { return guns[GunType::Assisted]; },
                                    .on_is_available = [](const ShopItem &) { return true; } });
    shop_items->push_back(ShopItem{ .name        = "Homing Gun",
                                    .description = "Follows enemies",
                                    .price       = 120,
                                    .on_accept =
                                      [this]
                                    {
                                      crystals -= 120;
                                      guns[GunType::Homing] = true;
                                    },
                                    .on_has_item     = [this](const ShopItem &) { return guns[GunType::Homing]; },
                                    .on_is_available = [](const ShopItem &) { return true; } });

    if (!shop_items->empty())
      gui->selected_index = 0;
  };
  action.on_update = [this, shop_items](Action &action)
  {
    gui->handle_selecting_index(gui->selected_index, shop_items->size() + 1);
    gui->handle_accepting_index(gui->selected_index,
                                [shop_items, &action](size_t index)
                                {
                                  TraceLog(LOG_TRACE, "Selected shop item: %zu / %zu", index, shop_items->size());
                                  if (index >= shop_items->size())
                                  {
                                    TraceLog(LOG_INFO, "Shop ended");
                                    action.is_done = true;
                                    return;
                                  }

                                  const auto &item = (*shop_items)[index];

                                  if (auto availability = item.is_available();
                                      availability == ShopItem::AvailabilityReason::Available)
                                  {
                                    TraceLog(LOG_INFO, "Buying shop item: %s", item.name.c_str());
                                    if (item.on_accept)
                                      item.on_accept();
                                  }
                                  else
                                  {
                                    TraceLog(LOG_INFO,
                                             "Shop item not available: %s (%s)",
                                             item.name.c_str(),
                                             magic_enum::enum_name(availability).data());
                                  }
                                });
  };
  action.on_draw = [this, shop_items](const Action &)
  {
    const auto &items = *shop_items;
    gui->draw_shop_items(items);
  };

  action.on_done = [this, shop_items](Action &)
  {
    gui->selected_index.reset();
    freeze_entities = false;
  };

  actions.push(std::move(action));
}

void Game::schedule_action_modify_ship(const Interactable *interactable) noexcept
{
  TraceLog(LOG_INFO, "Playing modify ship %p", (void *)interactable);

  std::shared_ptr<std::vector<ShopItem>> ship_items = std::make_shared<std::vector<ShopItem>>();

  Action action;
  action.on_start = [this, ship_items](Action &)
  {
    freeze_entities = true;

    if (guns[GunType::Normal])
      ship_items->push_back(ShopItem{ .name            = "Normal Gun",
                                      .description     = "Default weapon",
                                      .price           = 0,
                                      .on_accept       = [this] { gun = GunType::Normal; },
                                      .on_has_item     = [this](const ShopItem &) { return gun == GunType::Normal; },
                                      .on_is_available = [this](const ShopItem &) { return guns[GunType::Normal]; } });

    if (guns[GunType::Fast])
      ship_items->push_back(ShopItem{ .name            = "Fast Gun",
                                      .description     = "Shoots faster",
                                      .price           = 0,
                                      .on_accept       = [this] { gun = GunType::Fast; },
                                      .on_has_item     = [this](const ShopItem &) { return gun == GunType::Fast; },
                                      .on_is_available = [this](const ShopItem &) { return guns[GunType::Fast]; } });

    if (guns[GunType::Assisted])
      ship_items->push_back(
        ShopItem{ .name            = "Auto Gun",
                  .description     = "Auto-aims at enemies",
                  .price           = 0,
                  .on_accept       = [this] { gun = GunType::Assisted; },
                  .on_has_item     = [this](const ShopItem &) { return gun == GunType::Assisted; },
                  .on_is_available = [this](const ShopItem &) { return guns[GunType::Assisted]; } });

    if (guns[GunType::Homing])
      ship_items->push_back(ShopItem{ .name            = "Homing Gun",
                                      .description     = "Follows enemies",
                                      .price           = 0,
                                      .on_accept       = [this] { gun = GunType::Homing; },
                                      .on_has_item     = [this](const ShopItem &) { return gun == GunType::Homing; },
                                      .on_is_available = [this](const ShopItem &) { return guns[GunType::Homing]; } });

    if (!ship_items->empty())
      gui->selected_index = 0;
  };
  action.on_update = [this, ship_items](Action &action)
  {
    gui->handle_selecting_index(gui->selected_index, ship_items->size() + 1);
    gui->handle_accepting_index(
      gui->selected_index,
      [ship_items, &action](size_t index)
      {
        TraceLog(LOG_TRACE, "Selected modify ship item: %zu / %zu", index, ship_items->size());
        if (index >= ship_items->size())
        {
          TraceLog(LOG_INFO, "Modify ship ended");
          action.is_done = true;
          return;
        }

        const auto &item = (*ship_items)[index];

        if (auto availability = item.is_available(); availability == ShopItem::AvailabilityReason::Available)
        {
          TraceLog(LOG_INFO, "Selected modify ship item: %s", item.name.c_str());
          if (item.on_accept)
            item.on_accept();
        }
        else
        {
          TraceLog(LOG_INFO,
                   "Ship item not available: %s (%s)",
                   item.name.c_str(),
                   magic_enum::enum_name(availability).data());
        }
      });
  };
  action.on_draw = [this, ship_items](const Action &)
  {
    const auto &items = *ship_items;
    if (items.empty())
      return;
    gui->draw_ship_items(items);
  };

  action.on_done = [this, ship_items](Action &)
  {
    gui->selected_index.reset();
    freeze_entities = false;
  };

  actions.push(std::move(action));
}

void Game::schedule_action_ship_control(const Interactable *interactable) noexcept
{
  TraceLog(LOG_INFO, "Playing ship control %p", (void *)interactable);

  std::shared_ptr<std::vector<ShopItem>> ship_items = std::make_shared<std::vector<ShopItem>>();

  Action action;
  action.on_start = [this, ship_items, interactable](Action &)
  {
    freeze_entities = true;

    ship_items->push_back(ShopItem{ .name        = "Mission Select",
                                    .description = "",
                                    .price       = 0,
                                    .on_accept = [this, interactable] { schedule_action_mission_select(interactable); },
                                    .on_has_item     = [](const ShopItem &) { return false; },
                                    .on_is_available = [](const ShopItem &) { return true; } });

    ship_items->push_back(ShopItem{ .name            = "Change Weapons",
                                    .description     = "",
                                    .price           = 0,
                                    .on_accept       = [this] { schedule_action_modify_ship(nullptr); },
                                    .on_has_item     = [](const ShopItem &) { return false; },
                                    .on_is_available = [](const ShopItem &) { return true; } });

    if (!ship_items->empty())
      gui->selected_index = 0;
  };
  action.on_update = [this, ship_items](Action &action)
  {
    gui->handle_selecting_index(gui->selected_index, ship_items->size() + 1);
    gui->handle_accepting_index(gui->selected_index,
                                [ship_items, &action](size_t index)
                                {
                                  if (index >= ship_items->size())
                                  {
                                    action.is_done = true;
                                    return;
                                  }

                                  const auto &item = (*ship_items)[index];

                                  if (auto availability = item.is_available();
                                      availability == ShopItem::AvailabilityReason::Available)
                                  {
                                    if (item.on_accept)
                                      item.on_accept();

                                    action.is_done = true;
                                  }
                                });
  };
  action.on_draw = [this, ship_items](const Action &)
  {
    const auto &items = *ship_items;
    if (items.empty())
      return;
    gui->draw_ship_control(items);
  };

  action.on_done = [this, ship_items](Action &)
  {
    gui->selected_index.reset();
    freeze_entities = false;
  };

  actions.push(std::move(action));
}

void Game::schedule_action_mission_select(const Interactable *interactable) noexcept
{
  TraceLog(LOG_INFO, "Playing mission select action %p", (void *)interactable);

  std::shared_ptr<std::vector<ShopItem>> ship_items = std::make_shared<std::vector<ShopItem>>();

  Action action;
  action.on_start = [this, ship_items, interactable](Action &)
  {
    freeze_entities = true;

    for (const auto &[mnumber, mparams] : missions)
    {
      if (mparams.name.starts_with("_"))
      {
        TraceLog(LOG_INFO, "Skipping mission %s", mparams.name.c_str());
        continue;
      }

      ship_items->push_back(
        ShopItem{ .name        = mparams.name,
                  .description = mparams.description,
                  .price       = 0,
                  .on_accept   = [this, interactable, mnumber]
                  { schedule_action_change_level(Level::Asteroids, mnumber, interactable); },
                  .on_has_item     = [](const ShopItem &) { return false; },
                  .on_is_available = [unlocked = mparams.is_unlocked()](const ShopItem &) { return unlocked; } });
    }

    if (!ship_items->empty())
      gui->selected_index = 0;
  };
  action.on_update = [this, ship_items](Action &action)
  {
    gui->handle_selecting_index(gui->selected_index, ship_items->size() + 1);
    gui->handle_accepting_index(gui->selected_index,
                                [ship_items, &action](size_t index)
                                {
                                  if (index >= ship_items->size())
                                  {
                                    action.is_done = true;
                                    return;
                                  }

                                  const auto &item = (*ship_items)[index];

                                  if (auto availability = item.is_available();
                                      availability == ShopItem::AvailabilityReason::Available)
                                  {
                                    if (item.on_accept)
                                      item.on_accept();

                                    action.is_done = true;
                                  }
                                });
  };
  action.on_draw = [this, ship_items](const Action &)
  {
    const auto &items = *ship_items;
    if (items.empty())
      return;
    gui->draw_ship_control(items);
  };

  action.on_done = [this, ship_items](Action &)
  {
    gui->selected_index.reset();
    freeze_entities = false;
  };

  actions.push(std::move(action));
}

void Game::schedule_action_change_room(const Room::Type &room_type) noexcept
{
  TraceLog(LOG_INFO, "Changing room to %i", static_cast<int>(room_type));

  freeze_entities = true;

  // fade to black
  {
    Action action;
    action.on_update = [](Action &action)
    {
      action.data = std::get<float>(action.data) + DELTA_TIME * TRANSITION_SPEED * 3.0f;

      if (std::get<float>(action.data) >= 1.0f)
        action.is_done = true;
    };
    action.on_draw = [](const Action &action)
    {
      const float &data         = std::get<float>(action.data);
      const unsigned char alpha = 255.0f * data;
      DrawRectangle(0, 0, width, height, Color{ 0, 0, 0, alpha });
    };
    action.data = 0.0f;
    actions.push(std::move(action));
  }

  // change level
  {
    Action action;
    action.on_update = [this, room_type, old_room = room](Action &action)
    {
      auto new_room = Room::get(room_type);

      // NOTE: Player's position is relative to the room position,
      //       so we need to convert it to world coordinates
      //       and then back to the new room's coordinates
      player->position = position_to_room(position_to_world(player->position, old_room->rect), new_room->rect);

      set_room(room_type);

      action.is_done = true;
    };
    action.on_draw = [](const Action &) { DrawRectangle(0, 0, width, height, Color{ 0, 0, 0, 255 }); };
    actions.push(std::move(action));
  }

  // fade from black
  {
    Action action;
    action.on_update = [](Action &action)
    {
      action.data = std::get<float>(action.data) + DELTA_TIME * TRANSITION_SPEED * 2.0f;

      if (std::get<float>(action.data) >= 1.0f)
        action.is_done = true;
    };
    action.on_draw = [](const Action &action)
    {
      const float &data         = std::get<float>(action.data);
      const unsigned char alpha = 255.0f * (1.0f - data);
      DrawRectangle(0, 0, width, height, Color{ 0, 0, 0, alpha });
    };
    action.on_done = [this](Action &) { freeze_entities = false; };
    action.data    = 0.0f;
    actions.push(std::move(action));
  }
}

void Game::set_room(const Room::Type &room_type) noexcept
{
  room = Room::get(room_type);

  if (!room->tileset_name.empty() && (!tileset_sprite || tileset_sprite->get_path() != room->tileset_name))
    tileset_sprite = std::make_unique<Sprite>(room->tileset_name);

  TraceLog(LOG_INFO, "Room changed to %i", static_cast<int>(room_type));
}
