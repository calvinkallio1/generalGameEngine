# generalGameEngine

This project is based around SDL to create a framework on which to build games easily. It is created so that modules can easily be added to fit your needs, and has keyboard polling, graphics, and audio all included. It also has some basic quality-of-life features specifically for game development, detailed in the docs.

## Installation

First to install, you have to clone the library into your game repository. From here forward, the project will be referred to as *my_game*

```
mkdir my_game && cd my_game
git init 
git submodule add https://github.com/calvinkallio1/generalGameEngine vendored/engine
git submodule update --init --recursive
```

## Optimal Project Structure

The best project structure is as follows:

```
my_game/
  CMakeLists.txt 
  src/
    main.c 
    scenes.h 
    game_state.h 
    scene_title.c 
    scene_game.c 
    scene_pause.c 
  assets/
    sprites/ tiles/ fonts/ sfx/ music/
  vendored/
    engine/
```

This is the minimal project structure for a working game. What is in each file depends on what the build's needs are, all covered in docs. The game can be built out much further from there. Sample source code will be in docs.

## CMakeLists.txt

This is what should be in the CMakeLists file. It should not to be updated from here as long as project structure is maintained.

```
cmake_minimum_required(VERSION 3.21)
project(my_game LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Engine options: set BEFORE add_subdirectory. Both optional; PNG and fonts need them.
set(ENGINE_WITH_IMAGE ON CACHE BOOL "" FORCE)
set(ENGINE_WITH_TTF   ON CACHE BOOL "" FORCE)
add_subdirectory(vendored/engine)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
add_executable(${PROJECT_NAME} ${GAME_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE engine)   # brings in every engine header, SDL3, SDL_image, SDL_ttf
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

# Copy assets/ next to the executable so assets_texture("x.png") resolves.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)
```

### Build using CMake 

CMake is used to build out your game based off of your src folder and the game engine. First, you have to install CMake by following the directions on their website: https://cmake.org/download/

Here are the build commands to build and run your project, based off of the source files. The bare minimum here is a main.c, a game_state.c, scenes.h, and at least one scene file to get it to run. I recommend starting with a Title scene, as you only need to build it once and can edit it later.

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug 
cmake --build build -j 
./build/my_game #Runs the game
```



## TODO 

-Write docs
  -API reference
  -Demos
```
