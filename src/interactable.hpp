#pragma once

#include "mask.hpp"
#include "sprite.hpp"

class Interactable
{
public:
  virtual ~Interactable() = default;

  virtual void update() = 0;
  virtual void draw() const;
  virtual void interact() = 0;

  [[nodiscard]] const Sprite &get_sprite() const noexcept { return sprite; }

protected:
  mutable Sprite sprite{};
};

class Station final : public Interactable
{
public:
  Station();
  void update() override;
  void interact() override;

private:
};

class DialogEntity final : public Interactable
{
public:
  DialogEntity(const Vector2 &position);
  void update() override;
  void interact() override;
};
