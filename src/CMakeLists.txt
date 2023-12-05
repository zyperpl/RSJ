CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

ADD_SUBDIRECTORY(external)

INCLUDE_DIRECTORIES(external)
INCLUDE_DIRECTORIES(external/raylib/src)
INCLUDE_DIRECTORIES(external/raylib/src/external/glfw/include)

ADD_EXECUTABLE(${PROJECT_NAME}
  main.cpp 
  sound_manager.cpp
  sprite.cpp
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE raylib)
