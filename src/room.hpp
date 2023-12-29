#pragma once

#include <optional>
#include <string>

#include <raylib.h>
#include <raymath.h>

#include "interactable.hpp"
#include "mask.hpp"
#include "utils.hpp"

class Room;

class Room
{
public:
  enum class Type
  {
    DockingBay,
    MainHall,
    ControlRoom,
    EngineRoom,
    Armory,
    Laboratory,
    Workshop
  };

  std::unordered_map<Direction, std::shared_ptr<Room>> neighbours;
  Rectangle rect{ 0.0f, 0.0f, 0.0f, 0.0f };
  std::vector<std::unique_ptr<Interactable>> interactables;
  std::vector<Mask> masks;

  static void load();
  static void unload();

  [[nodiscard]] static std::shared_ptr<Room> get(const Type &type) noexcept;

private:
  static std::unordered_map<Type, std::shared_ptr<Room>> rooms;
};
