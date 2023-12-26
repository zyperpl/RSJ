#pragma once

#include "mask.hpp"
#include "sprite.hpp"

class Interactable
{
public:
  virtual ~Interactable() = default;

  virtual void update() = 0;
  virtual void draw() const;

  const Sprite &get_sprite() const noexcept { return sprite; }

protected:
  mutable Sprite sprite{};
};

class Station final : public Interactable
{
public:
  Station();
  void update() override;

private:
};

class DialogEntity final : public Interactable
{
public:
  DialogEntity(const Vector2 &position);
  void update() override;
};
