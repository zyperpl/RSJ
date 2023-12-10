#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <type_traits>

template<typename T, size_t BUFFER_SIZE>
struct ObjectCircularBuffer
{
  T objects[BUFFER_SIZE];
  size_t head{ 0 };
  size_t tail{ 0 };
  const size_t capacity{ BUFFER_SIZE };

  void push(T &&obj)
  {
    objects[head] = std::move(obj);
    head          = (head + 1) % BUFFER_SIZE;

    if (head == tail)
    {
      tail = (tail + 1) % BUFFER_SIZE;
    }
  }

  size_t size() const { return head >= tail ? head - tail : BUFFER_SIZE - tail + head; }

  bool empty() const { return head == tail; }
  bool full() const { return (head + 1) % BUFFER_SIZE == tail; }

  void clear()
  {
    head = 0;
    tail = 0;
  }

  void for_each(auto func)
  {
    const size_t start = tail;
    size_t end         = head >= tail ? head : head + BUFFER_SIZE;

    for (size_t i = start; i < end; i++)
    {
      const size_t index = i % BUFFER_SIZE;
      if constexpr (std::is_same_v<decltype(func(objects[index])), bool>)
      {
        const bool ret = func(objects[index]);
        if (!ret)
        {
          remove(index);

          end = head >= tail ? head : head + BUFFER_SIZE;
        }
      }
      else
      {
        func(objects[index]);
      }
    }
  }

  void remove(size_t index)
  {
    if (index >= BUFFER_SIZE)
      return;

    if (index == tail)
    {
      tail = (tail + 1) % BUFFER_SIZE;
    }
    else if (index == head)
    {
      head = (head - 1) % BUFFER_SIZE;
    }
    else
    {
      if (head > 0)
      {
        std::swap(objects[index], objects[head - 1]);
        head = (head - 1) % BUFFER_SIZE;
      }
      else
      {
        std::swap(objects[index], objects[BUFFER_SIZE - 1]);
        head = BUFFER_SIZE - 1;
      }
    }
  }
};
