#include "sound_manager.hpp"

std::unordered_map<std::string, SoundManager::Sound> SoundManager::sounds;
float SoundManager::volume{ 0.5f };
