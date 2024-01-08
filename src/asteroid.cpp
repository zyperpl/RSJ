#include "asteroid.hpp"

#include <cassert>

#include "bullet.hpp"
#include "game.hpp"
#include "particle.hpp"
#include "pickable.hpp"
#include "player.hpp"
#include "player_ship.hpp"
#include "utils.hpp"

#include "magic_enum/magic_enum.hpp"

static constexpr const float ASTEROIDS_SIZE[]{ 8.0f, 16.0f, 32.0f };
static constexpr const int ASTEROID_SPLIT_COUNT{ 2 };

std::unique_ptr<Sprite> Asteroid::ASTEROID_SPRITE{ nullptr };
std::unique_ptr<Sprite> Asteroid::ALIEN_SHIP_SPRITE{ nullptr };

Particle create_asteroid_particle(const Vector2 &position, unsigned char alpha = 255)
{
  const Vector2 pos{ position.x + static_cast<float>(GetRandomValue(-10, 10)),
                     position.y + static_cast<float>(GetRandomValue(-10, 10)) };
  const Vector2 vel = Vector2Normalize(
    Vector2{ static_cast<float>(GetRandomValue(-100, 100)), static_cast<float>(GetRandomValue(-100, 100)) });
  const float hue        = 229.0f - 10.0f + static_cast<float>(GetRandomValue(0, 20));
  const float saturation = 0.3f + static_cast<float>(GetRandomValue(0, 10)) / 100.0f;
  const float value      = 0.1f + static_cast<float>(GetRandomValue(0, 50)) / 100.0f;
  Color color            = ColorFromHSV(hue, saturation, value);
  color.a                = alpha;
  return Particle::create(pos, vel, color);
}

static std::unordered_map<uint8_t, Asteroid::Type> size_type_map{
  { 0, Asteroid::Type::Size1 },
  { 1, Asteroid::Type::Size2 },
  { 2, Asteroid::Type::Size3 },
};

static std::unordered_map<Asteroid::Type, std::string> type_tag_map{ { Asteroid::Type::Size1, "size1" },
                                                                     { Asteroid::Type::Size2, "size2" },
                                                                     { Asteroid::Type::Size3, "size3" },
                                                                     { Asteroid::Type::Crystal, "crystals" } };

[[nodiscard]] Asteroid Asteroid::create_normal(const Vector2 &position, uint8_t size)
{
  assert(size >= 0 && size < 3);
  assert(size < size_type_map.size());

  if (!ASTEROID_SPRITE)
    ASTEROID_SPRITE = std::make_unique<Sprite>("resources/asteroid.aseprite");

  const float speed_factor = 0.5f + (4.0f - static_cast<float>(size)) * 0.3f * 0.5f;
  const float random_angle = (static_cast<float>(GetRandomValue(0, 100)) / 100.0f) * M_PI * 2.0f;
  Asteroid asteroid;
  asteroid.position     = position;
  asteroid.velocity.x   = cos(random_angle) * speed_factor;
  asteroid.velocity.y   = sin(random_angle) * speed_factor;
  asteroid.type         = size_type_map[size];
  const float mask_size = ASTEROIDS_SIZE[size] * 0.5f;
  asteroid.mask.shapes.push_back(Circle{ Vector2{ 0.0f, 0.0f }, mask_size });
  return asteroid;
}

[[nodiscard]] Asteroid Asteroid::create_crystal(const Vector2 &position)
{
  if (!ASTEROID_SPRITE)
    ASTEROID_SPRITE = std::make_unique<Sprite>("resources/asteroid.aseprite");

  const float speed_factor = 0.5f + (4.0f - static_cast<float>(2)) * 0.3f * 0.3f;
  const float random_angle = (static_cast<float>(GetRandomValue(0, 100)) / 100.0f) * M_PI * 2.0f;
  Asteroid asteroid;
  asteroid.position     = position;
  asteroid.velocity.x   = cos(random_angle) * speed_factor;
  asteroid.velocity.y   = sin(random_angle) * speed_factor;
  asteroid.type         = Asteroid::Type::Crystal;
  const float mask_size = ASTEROIDS_SIZE[2] * 0.5f;
  asteroid.mask.shapes.push_back(Circle{ Vector2{ 0.0f, 0.0f }, mask_size });
  return asteroid;
}

[[nodiscard]] Asteroid Asteroid::create_alien_ship(const Vector2 &position)
{
  if (!ALIEN_SHIP_SPRITE)
    ALIEN_SHIP_SPRITE = std::make_unique<Sprite>("resources/alien_ship.aseprite");

  Asteroid asteroid;
  asteroid.position   = position;
  asteroid.velocity.x = 1.0f;
  asteroid.velocity.y = 0.0f;
  asteroid.type       = Asteroid::Type::AlienShip;
  asteroid.mask.shapes.push_back(Circle{ Vector2{ 0.0f, 0.0f }, 16.0f });
  return asteroid;
}

[[nodiscard]] Asteroid Asteroid::create_alien_bullet(const Vector2 &position, const Vector2 &direction)
{
  if (!ALIEN_SHIP_SPRITE)
    ALIEN_SHIP_SPRITE = std::make_unique<Sprite>("resources/alien_ship.aseprite");

  Asteroid asteroid;
  asteroid.position   = position;
  asteroid.velocity.x = direction.x * 2.0f;
  asteroid.velocity.y = direction.y * 2.0f;
  asteroid.type       = Asteroid::Type::AlienBullet;
  asteroid.mask.shapes.push_back(Circle{ Vector2{ 0.0f, 0.0f }, 4.0f });
  return asteroid;
}

bool Asteroid::update()
{
  position.x += velocity.x;
  position.y += velocity.y;

  wrap_position(position);

  GAME.bullets->for_each(
    [&](Bullet &bullet) -> bool
    {
      if (bullet.life <= 0)
        return true;

      const Mask bullet_mask{ bullet.position, { Circle{ Vector2{ 0.0f, 0.0f }, 5.0f } } };

      if (mask.check_collision(bullet_mask))
      {
        life--;

        for (int i = 0; i < 20; i++)
          GAME.particles->push(create_asteroid_particle(position, 100));

        bullet.life = 0;
        return false;
      }

      return true;
    });

  mask.position = position;

  if (life <= 0)
  {
    die();
    return false;
  }

  if (type == Type::AlienShip)
  {
    if (GAME.player)
    {
      if (GAME.frame % 240 == 0)
      {
        if (GetRandomValue(0, 1) == 0)
        {
          const Vector2 direction = Vector2Normalize(Vector2Subtract(GAME.player->position, position));
          GAME.asteroids->push(Asteroid::create_alien_bullet(position, direction));
        }
      }
    }
    velocity.y = sin(static_cast<float>(GAME.frame) / 100.0f) * 0.5f;
  }
  if (type == Type::AlienBullet)
  {
    if (GAME.frame % 10 == 0)
    {
      for (int i = 0; i < 10; i++)
        GAME.particles->push(Particle::create(position, Vector2Scale(velocity, 0.5f), ColorAlpha(RED, 0.9f)));
    }
    const int r = GetRandomValue(0, 60);
    if (GAME.frame % (180 + r) == 0)
    {
      life--;
    }
  }

  return true;
}

void Asteroid::die()
{
  if (type == Type::AlienShip)
  {
    GAME.score += 1000;
    for (int i = 0; i < 100; i++)
      GAME.particles->push(Particle::create(
        position,
        Vector2{ static_cast<float>(GetRandomValue(-1, 1)), static_cast<float>(GetRandomValue(-1, 1)) },
        Color{ 200, 255, 55, 250 }));
  }

  if (type == Type::AlienBullet)
    return;

  static SMSound sound1 = SoundManager::get("resources/explosion_asteroid1.wav");
  static SMSound sound2 = SoundManager::get("resources/explosion_asteroid2.wav");
  static SMSound sound3 = SoundManager::get("resources/explosion_asteroid3.wav");

  const uint8_t type_int = static_cast<uint8_t>(type);

  if (type_int <= 0)
    sound1.play();
  else if (type_int == 1)
    sound2.play();
  else
    sound3.play();

  if (type == Type::Size2 || type == Type::Size3)
  {
    for (size_t i = 0; i < ASTEROID_SPLIT_COUNT; i++)
    {
      GAME.asteroids->push(Asteroid::create_normal(position, type_int - 1));
    }

    int r = GetRandomValue(0, 100);
    if (r > 60)
    {
      int pickables_n = type_int;
      if (type_int >= 2)
      {
        r = GetRandomValue(0, 100);
        if (r < 10)
          pickables_n = 3;
        else if (r == 10)
          pickables_n = 4;
      }
      for (int i = 0; i < pickables_n; i++)
      {
        const Vector2 pos{ position.x + static_cast<float>(GetRandomValue(-4 * type_int, 4 * type_int)),
                           position.y + static_cast<float>(GetRandomValue(-3 * type_int, 3 * type_int)) };
        GAME.pickables->push(Pickable::create_ore(pos, Vector2Scale(velocity, 0.5f)));
      }
    }

    if (GAME.current_mission == 4 && QUEST("scientist1").is_accepted() && GAME.artifacts.empty())
    {
      if (GetRandomValue(0, 100) < 10)
      {
        bool found = false;
        GAME.pickables->for_each(
          [&](Pickable &pickable)
          {
            if (pickable.type == Pickable::Type::Artifact)
            {
              found = true;
              return;
            }
          });

        if (!found)
          GAME.pickables->push(Pickable::create_artifact(position, Vector2Scale(velocity, 0.9f)));
      }
    }
  }
  else if (type == Type::Crystal)
  {
    int pickables_n = 3 + GetRandomValue(1, 5);
    for (int i = 0; i < pickables_n; i++)
    {
      const Vector2 pos{ position.x + static_cast<float>(GetRandomValue(-4 * type_int, 4 * type_int)),
                         position.y + static_cast<float>(GetRandomValue(-3 * type_int, 3 * type_int)) };
      const Vector2 vel = Vector2Normalize(
        Vector2{ static_cast<float>(GetRandomValue(-100, 100)), static_cast<float>(GetRandomValue(-100, 100)) });
      GAME.pickables->push(Pickable::create_ore(pos, Vector2Add(vel, Vector2Scale(velocity, 0.5f))));
    }
  }

  for (int i = 0; i < 20 - std::max(1, type_int * 5); i++)
  {
    GAME.particles->push(create_asteroid_particle(position));
  }

  GAME.score += 100 * (3 - type_int);
}

void Asteroid::draw() const noexcept
{
  if (type == Type::AlienShip)
  {
    assert(ALIEN_SHIP_SPRITE);
    ALIEN_SHIP_SPRITE->set_centered();
    ALIEN_SHIP_SPRITE->position = position;

    draw_wrapped(ALIEN_SHIP_SPRITE->get_destination_rect(),
                 [&](const Vector2 &P)
                 {
                   ALIEN_SHIP_SPRITE->position = P;
                   ALIEN_SHIP_SPRITE->draw();
                 });
  }
  else if (type == Type::AlienBullet)
  {
    draw_wrapped(Rectangle{ position.x - 2.0f, position.y - 2.0f, 4.0f, 4.0f },
                 [&](const Vector2 &P) { DrawCircleV(P, 3.0f, RED); });
  }
  else
  {

    Color color = DARKPURPLE;
    assert(ASTEROID_SPRITE);
    assert(type_tag_map.find(type) != type_tag_map.end());

    ASTEROID_SPRITE->set_centered();
    ASTEROID_SPRITE->set_tag(type_tag_map[type]);
    ASTEROID_SPRITE->tint =
      ColorBrightness(color, 0.5f + static_cast<float>(life) / static_cast<float>(max_life) * 0.5f);
    ASTEROID_SPRITE->position = position;

    draw_wrapped(ASTEROID_SPRITE->get_destination_rect(),
                 [&](const Vector2 &P)
                 {
                   ASTEROID_SPRITE->position = P;
                   ASTEROID_SPRITE->draw();

                   if (CONFIG(show_masks))
                   {
                     Mask mask_copy     = mask;
                     mask_copy.position = P;
                     mask_copy.draw();
                   }
                 });
  }

  if (CONFIG(show_debug))
  {
    DrawPixelV(position, PINK);
    DrawText(TextFormat("%s", magic_enum::enum_name(type).data()), position.x, position.y, 10, RED);
  }
  if (CONFIG(show_velocity))
    DrawLineEx(position, Vector2{ position.x + velocity.x * 20.0f, position.y + velocity.y * 20.0f }, 1.0f, RED);
}

uint8_t Asteroid::size() const noexcept
{
  return static_cast<uint8_t>(type);
}
