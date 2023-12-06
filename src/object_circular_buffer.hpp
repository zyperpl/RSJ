#pragma once

#include <algorithm>
#include <cstddef>

constexpr size_t OBJECT_BUFFER_SIZE = 1024;

enum class ObjectState
{
  DEAD,
  ALIVE
};

template<typename T>
struct ObjectCircularBuffer
{
  T objects[OBJECT_BUFFER_SIZE];
  size_t head{ 0 };
  size_t tail{ 0 };

  void push(const T &obj)
  {
    objects[head] = obj;
    head          = (head + 1) % OBJECT_BUFFER_SIZE;
  }

  void pop() { tail = (tail + 1) % OBJECT_BUFFER_SIZE; }

  T &front() { return objects[tail]; }

  T &back() { return objects[head]; }

  size_t size() const { return head - tail; }

  bool empty() const { return head == tail; }

  void clear()
  {
    head = 0;
    tail = 0;
  }

  void update()
  {
    for (size_t i = tail; i < head; i++)
    {
      const ObjectState &state = objects[i].update();
      if (state == ObjectState::DEAD)
      {
        std::swap(objects[i], objects[head - 1]);
        head--;
      }
    }
  }

  void draw() const noexcept
  {
    for (size_t i = tail; i < head; i++)
    {
      objects[i].draw();
    }
  }
};
