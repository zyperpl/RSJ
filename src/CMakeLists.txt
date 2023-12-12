CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

ADD_SUBDIRECTORY(external)

INCLUDE_DIRECTORIES(external)
INCLUDE_DIRECTORIES(external/raylib/src)
INCLUDE_DIRECTORIES(external/raylib/src/external/glfw/include)

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
  ADD_COMPILE_OPTIONS(-ggdb)
  ADD_COMPILE_OPTIONS(-O0)
  ADD_COMPILE_OPTIONS(-DDEBUG)
ENDIF()

IF (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  ADD_COMPILE_OPTIONS(-ggdb)
ENDIF()

IF (CMAKE_BUILD_TYPE STREQUAL "Release")
  ADD_COMPILE_OPTIONS(-O3)
ENDIF()

ADD_EXECUTABLE(${PROJECT_NAME}
  asteroid.cpp
  bullet.cpp
  game.cpp
  main.cpp 
  mask.cpp
  particle.cpp
  pickable.cpp
  player.cpp
  render_pass.cpp
  resource.cpp
  sound_manager.cpp
  sprite.cpp
  utils.cpp
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE raylib)
