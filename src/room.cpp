#include "room.hpp"

#include <cassert>
#include <fstream>
#include <functional>
#include <string>
#include <unordered_map>

#include "ldtk.hpp"
#include "magic_enum/magic_enum.hpp"

#include "utils.hpp"

Vector2 position_to_world(const Vector2 &position, const Rectangle &rect)
{
  return Vector2{ position.x + rect.x, position.y + rect.y };
};
Vector2 position_to_room(const Vector2 &position, const Rectangle &rect)
{
  return Vector2{ position.x - rect.x, position.y - rect.y };
};

Tile tile_from_ldtk_tile(const ldtk::TileInstance &tile_instance, const float &tile_size)
{
  const float &x        = tile_instance.px[0];
  const float &y        = tile_instance.px[1];
  const float &source_x = tile_instance.src[0];
  const float &source_y = tile_instance.src[1];
  const float &w        = tile_size;
  const float &h        = tile_size;
  const Rectangle &source{ source_x, source_y, w, h };
  const Vector2 &pos{ x, y };
  const Vector2 &size{ w, h };

  return Tile{ pos, size, source };
}

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
  create_entity_from_name{
    { "NPC",
      [](const ldtk::EntityInstance &ldtk_entity) -> std::unique_ptr<Interactable>
      {
        const auto &entity_x = ldtk_entity.px[0];
        const auto &entity_y = ldtk_entity.px[1];

        const std::string name = get_field<std::string>(ldtk_entity.field_instances, "Name");

        auto entity =
          std::make_unique<DialogEntity>(Vector2{ static_cast<float>(entity_x), static_cast<float>(entity_y) }, name);
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
        entity->get_sprite().position = Vector2{ entity_x + entity_w / 2.0f, entity_y + entity_h / 2.0f };
        return entity;
      } }
  };

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

  std::unordered_map<std::string, std::shared_ptr<Room>> uid_room_map{};

  TraceLog(LOG_TRACE, "---");
  TraceLog(LOG_TRACE, "Loading %d rooms", ldtk.levels.size());
  for (const auto &level : ldtk.levels)
  {
    const auto &level_iid    = level.iid;
    const auto &level_name   = level.identifier;
    const auto &level_x      = level.world_x;
    const auto &level_y      = level.world_y;
    const auto &level_width  = level.px_wid;
    const auto &level_height = level.px_hei;

    auto room_type = magic_enum::enum_cast<Room::Type>(level_name);
    if (!room_type || !room_type.has_value())
    {
      TraceLog(LOG_ERROR, "Unknown room type: %s", level_name.c_str());
      continue;
    }

    TraceLog(LOG_TRACE, " > Loading room %s (enum value: %d)", level_name.c_str(), static_cast<int>(room_type.value()));

    auto room = std::make_shared<Room>();

    room->type = room_type.value();

    uid_room_map.emplace(level_iid, room);

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

        TraceLog(LOG_TRACE, "   > Loading colliders");
        for (size_t coord_id = 0; coord_id < layer.int_grid_csv.size(); coord_id++)
        {
          const auto &v = layer.int_grid_csv[coord_id];
          if (v == 0)
            continue;

          const auto &x = coord_id % layer.c_wid;
          const auto &y = coord_id / layer.c_wid;

          room->masks.emplace_back(Rectangle{ static_cast<float>(x * layer.grid_size + layer.grid_size / 2.0f),
                                              static_cast<float>(y * layer.grid_size + layer.grid_size / 2.0f),
                                              static_cast<float>(layer.grid_size),
                                              static_cast<float>(layer.grid_size) });
        }
        TraceLog(LOG_TRACE, "   > Loaded %d colliders", room->masks.size());

        TraceLog(LOG_TRACE, "   > Loading tiles");
        const auto &tile_size = layer.grid_size;
        if ((layer_name == "ForegroundTiles" || layer_name == "BackgroundTiles") && layer.tileset_rel_path.has_value())
        {
          room->tileset_name = get_resource_path(layer.tileset_rel_path.value());
          TraceLog(LOG_TRACE, "   > Tileset name: %s", room->tileset_name.c_str());
        }

        for (const auto &tile_instance : layer.grid_tiles)
        {
          if (layer_name == "ForegroundTiles")
            room->foreground_tiles.emplace_back(tile_from_ldtk_tile(tile_instance, tile_size));
          else if (layer_name == "BackgroundTiles")
            room->background_tiles.emplace_back(tile_from_ldtk_tile(tile_instance, tile_size));
        }
        TraceLog(LOG_TRACE, "   > Loaded %d background tiles", room->foreground_tiles.size());
        TraceLog(LOG_TRACE, "   > Loaded %d foreground tiles", room->background_tiles.size());
      }
    }

    rooms.emplace(room_type.value(), room);
  }

  for (const auto &level : ldtk.levels)
  {
    const auto &level_iid = level.iid;

    for (const auto &neighbour : level.neighbours)
    {
      if (!uid_room_map.contains(level_iid) || !uid_room_map.contains(neighbour.level_iid))
      {
        TraceLog(LOG_ERROR, "Failed to find room with uid %s or %s", level_iid.c_str(), neighbour.level_iid.c_str());
        assert(uid_room_map.contains(level_iid));
        assert(uid_room_map.contains(neighbour.level_iid));
        continue;
      }

      const auto &room           = uid_room_map[level_iid];
      const auto &neighbour_room = uid_room_map[neighbour.level_iid];
      Direction dir              = Direction::Down;
      if (neighbour.dir == "e")
        dir = Direction::Right;
      else if (neighbour.dir == "w")
        dir = Direction::Left;
      else if (neighbour.dir == "n")
        dir = Direction::Up;
      else if (neighbour.dir == "s")
        dir = Direction::Down;

      room->neighbours.emplace(dir, neighbour_room);
    }
  }

  TraceLog(LOG_TRACE, "Loaded %d rooms", rooms.size());
  TraceLog(LOG_TRACE, "---");

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
