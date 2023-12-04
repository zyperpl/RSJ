#pragma once

#include <array>
#include <memory>

class GameObject;

template<int N>
struct GameObjectCollection
{
  std::array<std::shared_ptr<GameObject>, N> objects;

  bool add(std::shared_ptr<GameObject> ptr)
  {
    GameObject *go_ptr = ptr.get();
    if (!go_ptr)
      return false;

    for (size_t i = 0; i < objects.size(); ++i)
    {
      if (objects[i] == nullptr)
      {
        objects[i] = ptr;
        return true;
      }
    }
    return false;
  }

  bool remove(std::shared_ptr<GameObject> ptr)
  {
    GameObject *go_ptr = ptr.get();
    if (!go_ptr)
      return false;

    for (size_t i = 0; i < objects.size(); ++i)
    {
      if (go_ptr == objects[i].get())
      {
        objects[i].reset();
        return true;
      }
    }
    return false;
  }

  bool remove(GameObject *ptr)
  {
    if (!ptr)
      return false;

    for (size_t i = 0; i < objects.size(); ++i)
    {
      if (ptr == objects[i].get())
      {
        objects[i].reset();
        return true;
      }
    }
    return false;
  }

  void clear()
  {
    for (size_t i = 0; i < objects.size(); ++i)
    {
      objects[i].reset();
    }
  }
};
