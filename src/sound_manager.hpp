#pragma once

#include <cassert>
#include <optional>
#include <string>
#include <unordered_map>

#include <raylib.h>

class SoundManager
{
public:
  struct Sound
  {
    Sound() = default;
    Sound(const Sound &other)
      : volume{ other.volume }
    {
      ray_sound = LoadSoundAlias(other.ray_sound);
    }
    Sound(Sound &&other) = delete;

    Sound(std::string_view path) { ray_sound = LoadSound(path.data()); }

    Sound &operator=(const Sound &other)
    {
      if (this == &other)
        return *this;

      volume    = other.volume;
      ray_sound = LoadSoundAlias(other.ray_sound);

      return *this;
    }

    Sound &operator=(Sound &&other) = delete;

    void play()
    {
      if (!IsSoundReady(ray_sound))
        return;

      if (volume)
        SetSoundVolume(ray_sound, *volume);
      else
        SetSoundVolume(ray_sound, SoundManager::volume);

      PlaySound(ray_sound);
    }

    void stop()
    {
      if (!IsSoundPlaying(ray_sound))
        return;

      StopSound(ray_sound);
    }

    void set_volume(float new_volume)
    {
      volume = new_volume;
      SetSoundVolume(ray_sound, new_volume);
    }

    bool is_playing() const { return IsSoundPlaying(ray_sound); }

    std::optional<float> volume;
    ::Sound ray_sound;
  };

  static SoundManager::Sound &get(std::string name)
  {
    if (sounds.contains(name))
      return sounds[name];

    sounds.insert({ name, SoundManager::Sound(name) });
    return sounds[name];
  }

  static SoundManager::Sound copy(std::string name)
  {
    if (sounds.contains(name))
      return Sound(sounds[name]);

    sounds.insert({ name, SoundManager::Sound(name) });
    return Sound(sounds[name]);
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

using SMSound = SoundManager::Sound;
