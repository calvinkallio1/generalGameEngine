# generalGameEngine

This project is based around SDL to create a framework on which to build games easily. It is created so that modules can easily be added to fit your needs, and has keyboard polling, graphics, and audio all included. It also has some basic quality-of-life features specifically for game development, detailed in the manual pages (see Documentation below).

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

This is the minimal project structure for a working game. What is in each file depends on what the build's needs are, all covered in the manual pages. The game can be built out much further from there. `vendored/engine/tools/gge init` creates this layout for you (see `gge man gge`).

## CMakeLists.txt

This is what should be in the CMakeLists file. It should not need to be updated from here as long as project structure is maintained.

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

## Architecture 

### The Loop 

Every frame, engine_run performs these actions:

- `poll_input` #Fill e->input from SDL events and device state
- `audio_update` #Keep music looping, sound effects 
- `handle_input` #Only on the top scene
- `apply_pending` #Perform queued operations 
- Update scenes
- `debug_flush`
- `present`

### Scenes and Scene Stack

A Scene is a structure of optional callbacks (`on_enter`, `on_exit`, `handle_input`, `update`, `render`), plus 2 flags (`blocks_render_below`, `blocks_update_below`). The engine itself can keep a stack of up to 8 Scenes. The scenes are pushed onto a stack of scenes, and each is looped through by the engine to update them. The request changes with push, pop, and replace methods. Each scene is its own file, and each scene must be included in `scenes.h`. Finally, the two flags determine whether or not the scenes *below* them on the stack update/render while that scene is on the top. For example, a pause scene might want to stop the lower scenes from updating, but not from rendering.

### Modules 

The engine is built based on modules, which can be easily configured/added to. Each module serves a specific purpose and has its own manual page.

## Documentation

The manual lives in `tools/man/` and is read with the `gge` tool (or plain `man` with `MANPATH` set, see `tools/README.md`):

```
vendored/engine/tools/gge man overview     # map of the engine: start here
vendored/engine/tools/gge man api          # every function, type and macro, by module
vendored/engine/tools/gge man scenes       # a module or concept page: scenes, state, entities, input,
                                           #   drawing, types, math, sprite, text, camera, tilemap, audio,
                                           #   menu, dialog, cutscene, tween, debug, assets, saves, build
vendored/engine/tools/gge man engine_push  # one page per function ...
vendored/engine/tools/gge man Camera       # ... per struct and enum ...
vendored/engine/tools/gge man CUT_DO       # ... and per macro
vendored/engine/tools/gge ls -m tilemap    # one-line summaries
```

Section-3 pages are generated from the `/** ... */` comments in the headers (`gge genman`); section-7 pages are hand-written.

## TODO 

- Demos
```
