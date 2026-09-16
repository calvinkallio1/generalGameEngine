# game_engine

A small 2D game engine in C on top of SDL3. It gives you a window, a fixed-timestep
loop, a per-frame input snapshot, and a scene stack. Everything game-specific lives
in your own code, which the engine reaches only through a `void *` it never reads.

```
src/engine/    the engine: engine.c/.h, input.h, scene.h  (built as a static library)
src/game/      empty here; a game's main.c and scenes go in this folder
vendored/SDL   SDL3 as a git submodule
docs/          TUTORIAL.md: how the engine works, and how to build a game on it
```

## Build

```
git clone --recurse-submodules https://github.com/calvinkallio1/generalGameEngine
cd generalGameEngine
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

If you cloned without `--recurse-submodules`, run `git submodule update --init` first.
The first build compiles SDL and takes a few minutes; after that only your files rebuild.
With `src/game/` empty this builds only the engine library. Add a `main.c` there and
the executable `build/game_engine` appears on the next build (see the tutorial for a
ten-line starting point).

## Two ways to start a game

Use this repo as a template: clone it, rename the project in `CMakeLists.txt`, and
put the game in `src/game/`. The `src/engine/` folder stays as is.

Or keep the engine separate: add this repo as a submodule of the game and link the
`engine` target. That one line brings in the engine's headers and SDL3.

```cmake
add_subdirectory(vendored/game_engine)
add_executable(my_game ${MY_SOURCES})
target_link_libraries(my_game PRIVATE engine)
```
