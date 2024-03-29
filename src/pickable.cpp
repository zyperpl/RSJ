#include "pickable.hpp"

#include <functional>

#include "game.hpp"
#include "particle.hpp"
#include "player.hpp"
#include "utils.hpp"

std::unique_ptr<Sprite> Pickable::ORE_SPRITE{};

Pickable Pickable::create(const Vector2 &position, const Vector2 &velocity, const std::function<void()> &func)
{
  if (!ORE_SPRITE)
    ORE_SPRITE = std::make_unique<Sprite>("resources/ore.aseprite");

  Pickable pickable(position, func);
  pickable.velocity = velocity;
  return pickable;
}

void ore_func()
{
  GAME.crystals += 1;
  GAME.score += 100;
}

Pickable Pickable::create_ore(const Vector2 &position, const Vector2 &velocity)
{
  auto pickable = create(position, velocity, ore_func);
  pickable.type = Type::Ore;
  return pickable;
}

Pickable Pickable::create_artifact(const Vector2 &position, const Vector2 &velocity)
{
  // TODO: better string building
  auto pickable =
    create(position,
           velocity,
           []() { GAME.artifacts.push(Artifact{ std::string("Artifact ") + std::to_string(GAME.current_mission) }); });
  pickable.type = Type::Artifact;
  return pickable;
}

Pickable::Pickable(const Vector2 &position, const std::function<void()> &func)
  : position{ position }
  , func{ func }
{
}

bool Pickable::update()
{
  position.x += velocity.x;
  position.y += velocity.y;

  mask.position = position;
  wrap_position(position);

  if (!GAME.player)
    return true;

  const auto &player = GAME.player;

  if (player_id == -1)
  {
    if (mask.check_collision(player->get_mask()))
    {
      player_id = 1;

      velocity = Vector2Normalize(Vector2Subtract(position, player->position));
      velocity.x *= 1.1f;
      velocity.y *= 1.1f;
    }
  }
  else
  {
    static SMSound sound = SoundManager::get("resources/pickup.wav");

    const float player_distance = Vector2Distance(position, player->position);
    if (player_distance < 8.0f)
    {
      sound.play();

      if (func)
        func();

      return false;
    }
    const Vector2 dir = Vector2Normalize(Vector2Subtract(player->position, position));

    if (player_distance > std::min(Game::width, Game::height) / 2.0f)
    {
      position.x = player->position.x - dir.x * 7.0f;
      position.y = player->position.y - dir.y * 7.0f;
    }

    velocity.x += dir.x * 0.1f;
    velocity.y += dir.y * 0.1f;

    position.x += dir.x * 0.5f;
    position.y += dir.y * 0.5f;
  }

  if (type == Type::Artifact)
  {
    if (GAME.frame % 2 == 0)
    {
      GAME.particles->push(Particle::create(position, Vector2{ 0.0f, 0.0f }, Color{ 10, 255, 255, 200 }));

      velocity.x += static_cast<float>(GetRandomValue(-10, 10)) / 1000.0f;
      velocity.y += static_cast<float>(GetRandomValue(-10, 10)) / 1000.0f;
    }
  }

  return true;
}

void Pickable::draw() const
{
  if (type == Type::Ore)
  {
    if (player_id != -1)
      ORE_SPRITE->scale = Vector2{ 0.86f, 0.86f };
    else
      ORE_SPRITE->scale = Vector2{ 1.0f, 1.0f };

    ORE_SPRITE->set_centered();
    ORE_SPRITE->position = position;
    draw_wrapped(ORE_SPRITE->get_destination_rect(),
                 [&](const Vector2 &position)
                 {
                   ORE_SPRITE->position = position;
                   ORE_SPRITE->draw();
                 });
  }
  else if (type == Type::Artifact)
  {
    Vector2 pos = position;
    if (GAME.frame % 5 == 0)
    {
      pos.x += static_cast<float>(GetRandomValue(-2, 2));
      pos.y += static_cast<float>(GetRandomValue(-2, 2));
    }

    draw_wrapped(Rectangle{ pos.x - 2.0f, pos.y - 2.0f, 4.0f, 4.0f },
                 [&](const Vector2 &position)
                 {
                   DrawPixelV(position, WHITE);
                   DrawRectangle(static_cast<int>(position.x), static_cast<int>(position.y), 4, 4, LIME);
                   DrawCircle(static_cast<int>(position.x), static_cast<int>(position.y), 2, SKYBLUE);
                   DrawText("?", static_cast<int>(position.x - 2.0f), static_cast<int>(position.y - 2.0f), 10, WHITE);
                 });
  }
}
