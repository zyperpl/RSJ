#include "room.hpp"

#include <cassert>
#include <fstream>
#include <functional>
#include <string>
#include <unordered_map>

#include "ldtk.hpp"
#include "magic_enum/magic_enum.hpp"

#include "utils.hpp"

std::unordered_map<Room::Type, std::shared_ptr<Room>> Room::rooms;

template<typename T>
T get_field(const std::vector<ldtk::FieldInstance> &fields, const std::string &name)
{
  auto field = std::find_if(std::begin(fields),
                            std::end(fields),
                            [&name](const ldtk::FieldInstance &field) { return field.identifier == name; });

  if (field == fields.end())
  {
    assert(field != fields.end());
    return T{};
  }

  const ldtk::FieldInstance &field_instance = *field;

  return field_instance.value.get<T>();
}

static std::unordered_map<std::string, std::function<std::unique_ptr<Interactable>(const ldtk::EntityInstance &)>>
  create_entity_from_name{ { "NPC",
                             [](const ldtk::EntityInstance &ldtk_entity) -> std::unique_ptr<Interactable>
                             {
                               const auto &entity_x = ldtk_entity.px[0];
                               const auto &entity_y = ldtk_entity.px[1];

                               const std::string name = get_field<std::string>(ldtk_entity.field_instances, "Name");

                               auto entity = std::make_unique<DialogEntity>(
                                 Vector2{ static_cast<float>(entity_x), static_cast<float>(entity_y) }, name);
                               return entity;
                             } },
                           { "DockedShip",
                             [](const ldtk::EntityInstance &ldtk_entity) -> std::unique_ptr<Interactable>
                             {
                               const float entity_x = static_cast<float>(ldtk_entity.px[0]);
                               const float entity_y = static_cast<float>(ldtk_entity.px[1]);
                               const float entity_w = static_cast<float>(ldtk_entity.width);
                               const float entity_h = static_cast<float>(ldtk_entity.height);

                               auto entity                   = std::make_unique<DockedShip>();
                               entity->get_sprite().position = Vector2{ entity_x, entity_y };
                               entity->get_sprite().scale.x  = entity_w / entity->get_sprite().get_width();
                               entity->get_sprite().scale.y  = entity_h / entity->get_sprite().get_height();
                               return entity;
                             } } };

void Room::load()
{
  std::ifstream file{ "resources/station.ldtk" };
  if (!file.is_open())
  {
    TraceLog(LOG_ERROR, "Failed to open level file!");
    return;
  }

  nlohmann::json ldtk_project_json;
  ldtk::Ldtk ldtk{};
  try
  {
    ldtk_project_json = nlohmann::json::parse(file);
    ldtk              = ldtk_project_json.get<ldtk::Ldtk>();
  }
  catch (const std::exception &e)
  {
    TraceLog(LOG_ERROR, "Failed to parse level file: %s", e.what());
    return;
  }

  std::unordered_map<int64_t, std::shared_ptr<Room>> uid_room_map{};

  TraceLog(LOG_TRACE, "---");
  TraceLog(LOG_TRACE, "Loading %d rooms", ldtk.levels.size());
  for (const auto &level : ldtk.levels)
  {
    const auto &level_uid    = level.uid;
    const auto &level_name   = level.identifier;
    const auto &level_x      = level.world_x;
    const auto &level_y      = level.world_y;
    const auto &level_width  = level.px_wid;
    const auto &level_height = level.px_hei;

    auto room_type = magic_enum::enum_cast<Room::Type>(level_name);
    if (!room_type)
    {
      TraceLog(LOG_ERROR, "Unknown room type: %s", level_name.c_str());
      continue;
    }

    TraceLog(LOG_TRACE, " > Loading room %s (enum value: %d)", level_name.c_str(), static_cast<int>(room_type.value()));

    auto room = std::make_shared<Room>();

    uid_room_map.emplace(level_uid, room);

    room->rect = Rectangle{ static_cast<float>(level_x),
                            static_cast<float>(level_y),
                            static_cast<float>(level_width),
                            static_cast<float>(level_height) };

    if (level.layer_instances.has_value())
    {
      const auto &layer_instances = level.layer_instances.value();
      for (const auto &layer : layer_instances)
      {
        const auto &layer_name = layer.identifier;

        TraceLog(LOG_TRACE, "  > Loading layer %s", layer_name.c_str());

        for (const auto &entity : layer.entity_instances)
        {
          const auto &entity_name = entity.identifier;

          TraceLog(LOG_TRACE, "   > Loading entity %s", entity_name.c_str());

          if (!create_entity_from_name.contains(entity_name))
          {
            TraceLog(LOG_ERROR, "   > Unknown entity type: %s", entity_name.c_str());
            assert(create_entity_from_name.contains(entity_name));
            continue;
          }
          auto interactable = create_entity_from_name[entity_name](entity);
          room->interactables.push_back(std::move(interactable));
        }

        for (size_t coord_id = 0; coord_id < layer.int_grid_csv.size(); coord_id++)
        {
          const auto &v = layer.int_grid_csv[coord_id];
          if (v == 0)
            continue;

          const auto &x = coord_id % layer.c_wid;
          const auto &y = coord_id / layer.c_wid;

          room->masks.emplace_back(Rectangle{ static_cast<float>(x * layer.grid_size),
                                              static_cast<float>(y * layer.grid_size),
                                              static_cast<float>(layer.grid_size),
                                              static_cast<float>(layer.grid_size) });
        }
      }
    }

    rooms.emplace(room_type.value(), room);
  }
  TraceLog(LOG_TRACE, "Loaded %d rooms", rooms.size());
  TraceLog(LOG_TRACE, "---");

  // TODO: neighbours

  file.close();
}

void Room::unload()
{
  rooms.clear();
}

std::shared_ptr<Room> Room::get(const Type &type) noexcept
{
  assert(rooms.contains(type));
  return rooms[type];
}
