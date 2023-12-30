#pragma once

#include <optional>
#include <string>

#include <raylib.h>
#include <raymath.h>

#include "interactable.hpp"
#include "mask.hpp"
#include "utils.hpp"

class Room;

[[nodiscard]] Vector2 position_to_world(const Vector2 &position, const Rectangle &rect);
[[nodiscard]] Vector2 position_to_room(const Vector2 &position, const Rectangle &rect);

struct Tile
{
  Vector2 position;
  Vector2 size;
  Rectangle source;
};

class Room
{
public:
  enum class Type
  {
    DockingBay,
    MainHall,
    ControlRoom,
    Corridor,
    EngineRoom,
    Armory,
    Laboratory,
    Workshop
  };

  Type type{ Type::DockingBay };
  std::unordered_map<Direction, std::shared_ptr<Room>> neighbours;
  Rectangle rect{ 0.0f, 0.0f, 0.0f, 0.0f };
  std::vector<std::unique_ptr<Interactable>> interactables;
  std::vector<Mask> masks;
  std::vector<Tile> foreground_tiles;
  std::vector<Tile> background_tiles;
  std::string tileset_name{};

  static void load();
  static void unload();

  [[nodiscard]] static std::shared_ptr<Room> get(const Type &type) noexcept;

private:
  static std::unordered_map<Type, std::shared_ptr<Room>> rooms;
};
