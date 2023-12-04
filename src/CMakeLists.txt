CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

ADD_SUBDIRECTORY(external)

INCLUDE_DIRECTORIES(external)
INCLUDE_DIRECTORIES(external/raylib/src)
INCLUDE_DIRECTORIES(external/raylib/src/external/glfw/include)

ADD_EXECUTABLE(executable 
  animation_sprite.cpp
  game.cpp
  input_manager.cpp
  main.cpp 
  player.cpp
  renderer.cpp
  sound_manager.cpp
  sprite.cpp
)

TARGET_LINK_LIBRARIES(executable PRIVATE raylib)
