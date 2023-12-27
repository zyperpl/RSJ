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
  SetTraceLogLevel(LOG_TRACE);

  set_state(GameState::PLAYING_STATION);

  TraceLog(LOG_TRACE, "Size of Asteroid buffer: %zukB", sizeof(Asteroid) * asteroids->capacity / 1024);
  TraceLog(LOG_TRACE, "Size of Bullet buffer: %zukB", sizeof(Bullet) * bullets->capacity / 1024);
  TraceLog(LOG_TRACE, "Size of Particle buffer: %zukB", sizeof(Particle) * particles->capacity / 1024);
  TraceLog(LOG_TRACE, "Size of Pickable buffer: %zukB", sizeof(Pickable) * pickables->capacity / 1024);

  for (size_t i = 0; i < stars.size(); i++)
    stars[i] = Vector2{ static_cast<float>(GetRandomValue(0, width)), static_cast<float>(GetRandomValue(0, height)) };

  TraceLog(LOG_TRACE, "Game initialized");
}

void Game::update()
{
  if (!actions.empty())
  {
    {
      Action &action = actions.front();
      action.update();
      if (action.done && action.on_done)
        action.on_done();
    }

    if (actions.front().done)
      actions.pop();
  }

  update_game();

  frame++;
}

void Game::update_game()
{
  if (!dialog)
  {
    for (auto &interactable : interactables)
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
      }
    }
  }

  if (dialog)
  {
    if (!dialog->responses.empty() && !selected_dialog_response_index.has_value())
      selected_dialog_response_index.value() = 0;

    if (selected_dialog_response_index.has_value())
    {
      size_t &response_index = selected_dialog_response_index.value();

      if (IsKeyPressed(KEY_DOWN))
      {
        response_index++;
        if (response_index >= dialog->responses.size())
          response_index = 0;
      }

      if (IsKeyPressed(KEY_UP))
      {
        if (response_index == 0)
          response_index = dialog->responses.size() - 1;
        else
          response_index--;
      }
    }

    if (IsKeyPressed(KEY_SPACE) || IsKeyPressedRepeat(KEY_SPACE))
    {
      TraceLog(LOG_TRACE, "Selected dialog response: %zu", selected_dialog_response_index.value());
      if (selected_dialog_response_index.has_value())
      {
        assert(selected_dialog_response_index.value() < dialog->responses.size());
        const auto &response       = dialog->responses[selected_dialog_response_index.value()];
        const auto &next_dialog_id = response.next_dialog_id;
        if (next_dialog_id.starts_with('_'))
        {
          TraceLog(LOG_WARNING, "Unknown dialog id: %s", next_dialog_id.c_str());
        }
        else
        {
          play_action(Action::Type::Dialog, next_dialog_id);
        }
      }

      dialog.reset();
      selected_dialog_response_index.reset();
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
        set_state(GameState::PLAYING_STATION);
      else
        set_state(GameState::PLAYING_ASTEROIDS);
    }

    if (IsKeyPressed(KEY_F4))
    {
      CONFIG(show_masks) = !CONFIG(show_masks);
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

  switch (state)
  {
    case GameState::PLAYING_ASTEROIDS:
    {
      for (const auto &interactable : interactables)
        interactable->draw();

      particles->for_each(std::bind(&Particle::draw, std::placeholders::_1));

      bullets->for_each(std::bind(&Bullet::draw, std::placeholders::_1));
      player->draw();
      asteroids->for_each(std::bind(&Asteroid::draw, std::placeholders::_1));
      pickables->for_each(std::bind(&Pickable::draw, std::placeholders::_1));
    }
    case GameState::PLAYING_STATION:
    {
      for (const auto &interactable : interactables)
        interactable->draw();

      particles->for_each(std::bind(&Particle::draw, std::placeholders::_1));
      pickables->for_each(std::bind(&Pickable::draw, std::placeholders::_1));

      player->draw();

      if (CONFIG(show_masks))
      {
        for (const auto &mask : masks)
          mask.draw();

        for (const auto &interactable : interactables)
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
  interactables.reserve(128);
  interactables.clear();
  masks.reserve(1024);
  masks.clear();

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

      interactables.push_back(std::make_unique<Station>());

      break;
    case GameState::PLAYING_STATION:
      player = std::make_unique<PlayerCharacter>();

      interactables.emplace_back(std::make_unique<DialogEntity>(Vector2{ width * 0.85f, height * 0.5f }));

      masks.push_back(Mask{ Rectangle{ width * 0.35f - 8.0f, height * 0.5f + 32.0f, 16.0f, 16.0f } });
      break;
    case GameState::GAME_OVER:
      break;
    default:
      break;
  }
}
