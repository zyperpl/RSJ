#include "pickable.hpp"

#include "game.hpp"
#include "player.hpp"
#include "utils.hpp"

Pickable::Pickable(Sprite &&sprite, const Vector2 &position, std::function<void()> func)
  : sprite{ std::move(sprite) }
  , position{ position }
  , velocity{ 0.0f, 0.0f }
  , func{ func }
{
  sprite.set_frame(0);
  sprite.set_centered();
}

bool Pickable::update()
{
  position.x += velocity.x;
  position.y += velocity.y;

  wrap_position(position);

  mask.position = position;

  if (Game::get().player)
  {
    const Player &player   = *Game::get().player;
    const Mask player_mask = player.mask;

    if (mask.check_collision(player_mask))
    {
      if (func)
        func();
      return false;
    }
  }

  return true;
}

void Pickable::draw() const
{
  if (IsTextureReady(sprite.get_texture()))
  {
    draw_wrapped(sprite.get_destination_rect(),
                 [&](const Vector2 &position)
                 {
                   sprite.position.x = position.x;
                   sprite.draw();
                 });
  }

  if (Game::get().CONFIG.show_debug)
  {
    draw_wrapped(Rectangle{ position.x, position.y, 10.0f, 10.0f },
                 [&](const Vector2 &position)
                 {
                   mask.position = position;
                   mask.draw();
                 });
  }
}
