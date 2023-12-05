#include <cassert>
#include <cmath>
#include <functional>
#include <memory>

#include <GLFW/glfw3.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#if defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>
#endif

static float accumulator = 0.0f;

const int game_width  = 480;
const int game_height = 270;

const int window_width  = game_width;
const int window_height = game_height;

const bool integer_scaling = true;

const float delta_time = 1.0f / 60.0f;

#define FRAMES(n) (delta_time * (float)(n))

#include "sprite.hpp"
#include "timer.hpp"

struct Bullet;
struct Player;
struct Game;
struct RenderPass;

template<typename T>
struct ObjectCircularBuffer;

static std::unique_ptr<Game> game;

Rectangle texture_rect(const Texture2D &texture)
{
  return Rectangle{ 0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height) };
}

Rectangle texture_rect_flipped(const Texture2D &texture)
{
  return Rectangle{
    static_cast<float>(texture.width), 0.0f, static_cast<float>(texture.width), static_cast<float>(-texture.height)
  };
}

void wrap_position(Vector2 &position)
{
  if (position.x < 0)
    position.x = game_width - 1;

  if (position.x >= game_width)
    position.x = 0;

  if (position.y < 0)
    position.y = game_height - 1;

  if (position.y >= game_height)
    position.y = 0;
}

struct RenderPass
{
  RenderTexture2D render_texture{};
  std::function<void()> render_func;

  RenderPass(int width, int height)
  {
    render_texture = LoadRenderTexture(width, height);
    assert(IsRenderTextureReady(render_texture));
  }

  ~RenderPass() { UnloadRenderTexture(render_texture); }

  void render()
  {
    BeginTextureMode(render_texture);
    {
      ClearBackground(BLACK);
      render_func();
    }
    EndTextureMode();
  }

  void draw(const Rectangle &render_destination)
  {
    DrawTexturePro(render_texture.texture,
                   texture_rect_flipped(render_texture.texture),
                   render_destination,
                   Vector2Zero(),
                   0.0f,
                   WHITE);
  }
};

enum class State
{
  DEAD,
  ALIVE
};

struct Asteroid
{
  Vector2 position{};
  Vector2 velocity{};
  float rotation_speed{ 0.0f };
  float rotation{ 0.0f };
  float size{ 24.0f };

  State update();

  void draw() const noexcept { 
    DrawCircleLines(position.x, position.y, size, RED);
    DrawCircleLines(position.x + game_width, position.y, size, RED);
    DrawCircleLines(position.x - game_width, position.y, size, RED);
    DrawCircleLines(position.x, position.y + game_height, size, RED);
    DrawCircleLines(position.x, position.y - game_height, size, RED);
  }
};

struct Bullet
{
  Vector2 position{};
  Vector2 velocity{};
  uint8_t life{ 40 };

  State update()
  {
    if (life == 0)
      return State::DEAD;

    position.x += velocity.x;
    position.y += velocity.y;

    wrap_position(position);

    life--;

    return State::ALIVE;
  }

  void draw() const noexcept { DrawCircle(position.x, position.y, 2.0f, PINK); }
};

constexpr size_t OBJECT_BUFFER_SIZE = 1024;

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
      State state = objects[i].update();
      if (state == State::DEAD)
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

struct Player
{
  Sprite sprite{ "resources/test.aseprite", "idle" };
  Vector2 position{ game_width / 2.0f, game_height / 2.0f };
  Vector2 velocity{ 0.0f, 0.0f };

  float rotation_speed{ 3.0f };
  float acceleration_speed{ 0.05f };

  Timer shoot_timer{ FRAMES(15) };

  void update();

  void draw() noexcept
  {
    sprite.origin = Vector2{ sprite.get_width() / 2.0f, sprite.get_height() / 2.0f };

    sprite.position.x = position.x + game_width;
    sprite.position.y = position.y;
    sprite.draw();

    sprite.position.x = position.x - game_width;
    sprite.position.y = position.y;
    sprite.draw();

    sprite.position.x = position.x;
    sprite.position.y = position.y - game_height;
    sprite.draw();

    sprite.position.x = position.x;
    sprite.position.y = position.y + game_height;
    sprite.draw();

    sprite.position.x = position.x;
    sprite.position.y = position.y;
    sprite.draw();

    DrawCircle(sprite.position.x, sprite.position.y, 2.0f, RED);
  }
};

struct Game
{
  std::unique_ptr<Player> player;
  std::unique_ptr<ObjectCircularBuffer<Bullet>> bullets;
  std::unique_ptr<ObjectCircularBuffer<Asteroid>> asteroids;

  void init()
  {
    player    = std::make_unique<Player>();
    bullets   = std::make_unique<ObjectCircularBuffer<Bullet>>();
    asteroids = std::make_unique<ObjectCircularBuffer<Asteroid>>();

    for (size_t i = 0; i < 10; i++)
    {
      Asteroid asteroid;
      asteroid.position.x     = GetRandomValue(0, game_width);
      asteroid.position.y     = GetRandomValue(0, game_height);
      asteroid.velocity.x     = GetRandomValue(-1, 1);
      asteroid.velocity.y     = GetRandomValue(-1, 1);
      asteroid.rotation_speed = GetRandomValue(-1, 1) * 0.1f;
      asteroids->push(asteroid);
    }
  }

  void update()
  {
    player->update();
    bullets->update();
    asteroids->update();
  }

  void draw() noexcept
  {
    player->draw();
    bullets->draw();
    asteroids->draw();
  }
};

State Asteroid::update()
{
  position.x += velocity.x;
  position.y += velocity.y;

  wrap_position(position);

  rotation += rotation_speed;

  for (size_t i = game->bullets->tail; i < game->bullets->head; i++)
  {
    Bullet &bullet = game->bullets->objects[i];
    if (bullet.life <= 0)
      continue;

    if (CheckCollisionCircles(position, size, bullet.position, 2.0f))
    {
      bullet.life = 0;

      if (size > 10.0f)
      {
        const float r1 = GetRandomValue(0, 360) * DEG2RAD;
        const float r2 = GetRandomValue(0, 360) * DEG2RAD;
        const float r3 = GetRandomValue(0, 360) * DEG2RAD;
        const float r4 = GetRandomValue(0, 360) * DEG2RAD;

        Asteroid asteroid1;
        asteroid1.position       = position;
        asteroid1.velocity.x     = cos(r1 + rotation * DEG2RAD + M_PI / 2.0f + M_PI) * 2.0f;
        asteroid1.velocity.y     = sin(r2 + rotation * DEG2RAD + M_PI / 2.0f + M_PI) * 2.0f;
        asteroid1.rotation_speed = GetRandomValue(-1, 1) * 0.01f;
        asteroid1.size           = size * 0.5f;

        Asteroid asteroid2;
        asteroid2.position       = position;
        asteroid2.velocity.x     = cos(r3 + rotation * DEG2RAD + M_PI / 2.0f + M_PI) * 2.0f;
        asteroid2.velocity.y     = sin(r4 + rotation * DEG2RAD + M_PI / 2.0f + M_PI) * 2.0f;
        asteroid2.rotation_speed = GetRandomValue(-1, 1) * 0.01f;
        asteroid2.size           = size * 0.5f;

        game->asteroids->push(asteroid1);
        game->asteroids->push(asteroid2);
      }

      return State::DEAD;
    }
  }

  return State::ALIVE;
}

void Player::update()
{
  sprite.animate();

  if (IsKeyDown(KEY_SPACE))
    sprite.set_tag("walk");
  else
    sprite.set_tag("idle");

  if (IsKeyDown(KEY_LEFT))
    sprite.rotation -= 1.0f * rotation_speed;

  if (IsKeyDown(KEY_RIGHT))
    sprite.rotation += 1.0f * rotation_speed;

  if (IsKeyDown(KEY_UP))
    velocity.y -= 1.0f;

  if (IsKeyDown(KEY_DOWN))
    velocity.y += 1.0f;

  shoot_timer.update(delta_time);

  if (IsKeyDown(KEY_SPACE) && shoot_timer.is_done())
  {
    Bullet bullet;
    bullet.position   = position;
    bullet.velocity.x = cos(sprite.rotation * DEG2RAD + M_PI / 2.0f + M_PI) * 5.0f;
    bullet.velocity.y = sin(sprite.rotation * DEG2RAD + M_PI / 2.0f + M_PI) * 5.0f;
    game->bullets->push(bullet);

    shoot_timer.start();
  }

  position.x += cos(sprite.rotation * DEG2RAD + M_PI / 2.0f) * velocity.y * acceleration_speed;
  position.y += sin(sprite.rotation * DEG2RAD + M_PI / 2.0f) * velocity.y * acceleration_speed;

  velocity.x *= 0.99f;
  velocity.y *= 0.99f;

  wrap_position(position);
}

void update_draw_frame()
{
  static RenderPass render_pass(game_width, game_height);
  if (!game)
  {
    game = std::make_unique<Game>();
    game->init();
  }

  if (!render_pass.render_func)
  {
    render_pass.render_func = [&]() { game->draw(); };
  }

  const float screen_width_float  = static_cast<float>(GetScreenWidth());
  const float screen_height_float = static_cast<float>(GetScreenHeight());

  float scale =
    std::max(std::min(screen_width_float / (float)(game_width), screen_height_float / (float)(game_height)), 1.0f);
  if (integer_scaling)
    scale = std::floor(scale);

  Rectangle render_destination;
  render_destination.x      = (screen_width_float - (game_width * scale)) * 0.5f;
  render_destination.y      = (screen_height_float - (game_height * scale)) * 0.5f;
  render_destination.width  = game_width * scale;
  render_destination.height = game_height * scale;

  const float interval = delta_time;
  size_t steps         = 6;
  const float dt       = GetFrameTime();
  accumulator += dt;

  auto update = [&]() { game->update(); };

  while (accumulator >= interval)
  {
    accumulator -= interval;

    update();

    steps--;
    if (steps == 0)
    {
      accumulator = 0;
    }
  }

  static Camera2D camera{};
  camera.target   = { 0.0f, 0.0f };
  camera.target   = Vector2{ game_width / 2.0f, game_height / 2.0f };
  camera.offset   = Vector2{ game_width / 2.0f, game_height / 2.0f };
  camera.rotation = 0.0f;
  camera.zoom     = 1.0f;

  BeginDrawing();
  {
    render_pass.render();

    ClearBackground(RAYWHITE);
    render_pass.draw(render_destination);
  }
  EndDrawing();
  glFinish();
}

int main(void)
{
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(window_width, window_height, "ASTEROIDS");
  SetWindowState(FLAG_VSYNC_HINT);
  SetExitKey(KEY_NULL);
  InitAudioDevice();
  SetTargetFPS(60);

#if defined(EMSCRIPTEN)
  emscripten_set_main_loop(update_draw_frame, 0, 1);
#else
  while (!WindowShouldClose())
  {
    update_draw_frame();

    if (IsKeyPressed(KEY_ESCAPE))
      CloseWindow();
  }
#endif

  CloseAudioDevice();

  if (IsWindowReady())
    CloseWindow();

  return 0;
}
