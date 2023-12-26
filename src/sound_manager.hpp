#pragma once

#include <cassert>
#include <string>
#include <unordered_map>

#include <raylib.h>

class SoundManager
{
public:
  struct Sound
  {
    Sound()                   = default;
    Sound(const Sound &other) = default;
    Sound(std::string_view path) { ray_sound = LoadSound(path.data()); }

    Sound &operator=(const Sound &) = default;

    void play() const
    {
      if (!IsSoundReady(ray_sound))
        return;

      SetSoundVolume(ray_sound, SoundManager::volume);
      PlaySound(ray_sound);
    }

    mutable ::Sound ray_sound;
  };

  static const SoundManager::Sound &get_sound(std::string name)
  {
    if (sounds.contains(name))
      return sounds[name];

    sounds.insert({ name, SoundManager::Sound(name) });
    return sounds[name];
  }

  static void clear()
  {
    for (auto &[name, sound] : sounds)
      UnloadSound(sound.ray_sound);
    sounds.clear();
  }

  static float volume;

private:
  static std::unordered_map<std::string, SoundManager::Sound> sounds;
};
