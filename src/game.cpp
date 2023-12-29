#include "game.hpp"

#include <cassert>

#include <raylib.h>
#include <raymath.h>

#include "asteroid.hpp"
#include "bullet.hpp"
#include "interactable.hpp"
#include "object_circular_buffer.hpp"
#include "particle.hpp"
#include "pickable.hpp"
#include "player_character.hpp"
#include "player_ship.hpp"
#include "room.hpp"
#include "utils.hpp"

Config Game::config{};
uint64_t Game::frame{ 0 };

Game &Game::get() noexcept
{
  static Game game;
  return game;
}

void Game::init()
{
  font        = LoadFontEx("resources/Kenney Mini Square.ttf", 10, nullptr, 0);
  dialog_font = LoadFontEx("resources/Kenney Mini.ttf", 10, nullptr, 0);

  Room::load();
  room = Room::get(Room::Type::DockingBay);

  set_state(GameState::PLAYING_STATION);
  set_room(Room::Type::DockingBay);

  TraceLog(LOG_TRACE, "Size of Asteroid buffer: %zukB", sizeof(Asteroid) * asteroids->capacity / 1024);
  TraceLog(LOG_TRACE, "Size of Bullet buffer: %zukB", sizeof(Bullet) * bullets->capacity / 1024);
  TraceLog(LOG_TRACE, "Size of Particle buffer: %zukB", sizeof(Particle) * particles->capacity / 1024);
  TraceLog(LOG_TRACE, "Size of Pickable buffer: %zukB", sizeof(Pickable) * pickables->capacity / 1024);

  for (size_t i = 0; i < stars.size(); i++)
    stars[i] = Vector2{ static_cast<float>(GetRandomValue(0, width)), static_cast<float>(GetRandomValue(0, height)) };

  quests.clear();
  quests.emplace("captain1",
                 Quest{ .description  = "Collect 10 crystals",
                        .progress     = []() { return GAME.crystals; },
                        .max_progress = []() { return 10; },
                        .on_report    = []() { GAME.crystals -= 10; } });

  quests.emplace("archeologist1",
                 Quest{ .description  = "Collect ancient artifact",
                        .progress     = []() { return GAME.artifacts.size(); },
                        .max_progress = []() { return 1; },
                        .on_report    = []() { GAME.artifacts.pop(); } });

  TraceLog(LOG_TRACE, "Game initialized");
}

Game::~Game() noexcept
{
  Room::unload();

  UnloadFont(font);
  UnloadFont(dialog_font);
}

void Game::update()
{
  if (!actions.empty())
  {
    {
      Action &action = actions.front();

      if (!action.has_started)
      {
        action.start();
        action.has_started = true;
      }

      action.update();
      if (action.is_done)
        action.done();
    }

    if (actions.front().is_done)
    {
      actions.pop();
      return;
    }
  }

  update_game();

  frame++;
}

void Game::update_game()
{
  assert(room);

  if (!dialog)
  {
    for (auto &interactable : room->interactables)
      interactable->update();

    if (state == GameState::PLAYING_ASTEROIDS)
    {
      if (!freeze_entities)
      {
        player->update();
        bullets->for_each(std::bind(&Bullet::update, std::placeholders::_1));
        asteroids->for_each(std::bind(&Asteroid::update, std::placeholders::_1));
        pickables->for_each(std::bind(&Pickable::update, std::placeholders::_1));
      }

      particles->for_each(std::bind(&Particle::update, std::placeholders::_1));

      update_background();
    }

    if (state == GameState::PLAYING_STATION)
    {
      if (!freeze_entities)
      {
        player->update();

        // TODO: check room bounds
      }
    }
  }

#if defined(DEBUG)
  if (IsKeyDown(KEY_LEFT_SHIFT))
  {
    if (IsKeyPressed(KEY_F1) && asteroids)
    {
      for (size_t i = 0; i < NUMBER_OF_ASTEROIDS; i++)
      {
        const Vector2 position = { static_cast<float>(GetRandomValue(0, width)),
                                   static_cast<float>(GetRandomValue(0, height)) };
        asteroids->push(Asteroid::create(position, 2));
      }
    }

    if (IsKeyPressed(KEY_F2) && asteroids)
    {
      asteroids->for_each([](Asteroid &asteroid) { asteroid.life = 0; });
    }

    if (IsKeyPressed(KEY_F3))
    {
      if (state == GameState::PLAYING_ASTEROIDS)
      {
        set_state(GameState::PLAYING_STATION);
        set_room(Room::Type::DockingBay);
      }
      else
        set_state(GameState::PLAYING_ASTEROIDS);
    }

    if (IsKeyPressed(KEY_F4))
    {
      CONFIG(show_masks) = !CONFIG(show_masks);
    }

    if (IsKeyPressed(KEY_F5))
    {
      static int room = 0;
      room++;
      set_room(static_cast<Room::Type>(room % static_cast<int>(Room::Type::Workshop)));
    }
  }
#endif
}

void Game::update_background() noexcept
{
  for (size_t i = 0; i < stars.size(); i++)
  {
    stars[i].x += 0.1f;
    if (i % 2 == 0)
      stars[i].x += 0.2f;

    if (stars[i].x > width)
    {
      stars[i].x -= width;
      stars[i].y = static_cast<float>(GetRandomValue(0, height));
    }
  }
}

void Game::draw() noexcept
{
  draw_background();

  assert(room);

  switch (state)
  {
    case GameState::PLAYING_ASTEROIDS:
    {
      for (const auto &interactable : room->interactables)
        interactable->draw();

      particles->for_each(std::bind(&Particle::draw, std::placeholders::_1));

      bullets->for_each(std::bind(&Bullet::draw, std::placeholders::_1));
      player->draw();
      asteroids->for_each(std::bind(&Asteroid::draw, std::placeholders::_1));
      pickables->for_each(std::bind(&Pickable::draw, std::placeholders::_1));
    }
    case GameState::PLAYING_STATION:
    {
      for (const auto &interactable : room->interactables)
        interactable->draw();

      particles->for_each(std::bind(&Particle::draw, std::placeholders::_1));
      pickables->for_each(std::bind(&Pickable::draw, std::placeholders::_1));

      player->draw();

#if defined(DEBUG)
      // TODO: remove mask drawing
      for (const auto &mask : room->masks)
        mask.draw();
#endif

      if (CONFIG(show_masks))
      {
        for (const auto &mask : room->masks)
          mask.draw();

        for (const auto &interactable : room->interactables)
          Mask(interactable->get_sprite().get_destination_rect()).draw();
      }
    }
    case GameState::GAME_OVER:
      break;
    case GameState::MENU:
      break;
  }

  if (!actions.empty())
  {
    const Action &action = actions.front();
    action.draw();
  }
}

void Game::draw_background() noexcept
{
  for (size_t i = 0; i < stars.size(); i++)
  {
    const Vector2 &star = stars[i];
    if (i % 2 == 0)
      DrawPixel(star.x, star.y, Color{ 240, 180, 100, 255 });
    else
      DrawPixel(star.x, star.y, Color{ 120, 230, 100, 255 });
  }

  static auto asteroid_sprite = Sprite{ "resources/asteroid.aseprite" };
  asteroid_sprite.set_frame(1);
  asteroid_sprite.tint = ColorBrightness(BLACK, 0.2f);
  const long &w        = static_cast<long>(asteroid_sprite.get_width());
  const long &h        = static_cast<long>(asteroid_sprite.get_height());

  for (int x = -w; x <= width + w; x += asteroid_sprite.get_width())
  {
    for (int y = -h; y <= height + h; y += asteroid_sprite.get_height())
    {
      if ((x * y) % 3 == 0 || (x + y) % 5 == 0 || (x * y) % 7 == 0 || (x + y) % 9 == 0)
        continue;

      float xf = static_cast<float>(x) - sin(frame * 0.001f + x * 37.542f) * static_cast<float>(w) * 0.5f;
      float yf = static_cast<float>(y) - cos(frame * 0.002f - y * 13.127f) * static_cast<float>(h) * 0.8f;
      asteroid_sprite.position = Vector2{ xf, yf };
      asteroid_sprite.set_frame((x + y - 1) % 3);
      asteroid_sprite.draw();
    }
  }
}

void Game::set_state(GameState new_state) noexcept
{
  state = new_state;

  bullets   = std::make_unique<ObjectCircularBuffer<Bullet, 128>>();
  asteroids = std::make_unique<ObjectCircularBuffer<Asteroid, 2048>>();
  particles = std::make_unique<ObjectCircularBuffer<Particle, 4096>>();
  pickables = std::make_unique<ObjectCircularBuffer<Pickable, 1024>>();

  switch (state)
  {
    case GameState::PLAYING_ASTEROIDS:
      player = std::make_unique<PlayerShip>();

      for (size_t i = 0; i < NUMBER_OF_ASTEROIDS; i++)
      {
        const Vector2 position = { static_cast<float>(GetRandomValue(0, width)),
                                   static_cast<float>(GetRandomValue(0, height)) };
        asteroids->push(Asteroid::create(position, 2));
      }

      for (size_t i = 0; i < 10; i++)
      {
        Vector2 particle_position{ static_cast<float>(GetRandomValue(0, width)),
                                   static_cast<float>(GetRandomValue(0, height)) };
        Vector2 particle_velocity{ static_cast<float>(GetRandomValue(-100, 100)) / 100.0f,
                                   static_cast<float>(GetRandomValue(-100, 100)) / 100.0f };
        Color particle_color{ static_cast<unsigned char>(GetRandomValue(0, 255)),
                              static_cast<unsigned char>(GetRandomValue(0, 255)),
                              static_cast<unsigned char>(GetRandomValue(0, 255)),
                              static_cast<unsigned char>(GetRandomValue(0, 255)) };
        particles->push(Particle::create(particle_position, particle_velocity, particle_color));
      }

      for (size_t i = 0; i < 2; i++)
      {
        const Vector2 position{ static_cast<float>(GetRandomValue(0, width)),
                                static_cast<float>(GetRandomValue(0, height)) };
        pickables->push(Pickable::create_ore(position, Vector2Zero()));
      }

      room = std::make_shared<Room>(); // XXX: Asteroids room is not loaded from file
      room->interactables.push_back(std::make_unique<Station>());

      break;
    case GameState::PLAYING_STATION:
      player = std::make_unique<PlayerCharacter>();
      break;
    case GameState::GAME_OVER:
      break;
    default:
      break;
  }
}
