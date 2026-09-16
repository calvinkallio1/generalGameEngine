cmake_minimum_required(VERSION 3.16)
project(cat_game LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # clangd / editor support

# ---- SDL3 (vendored, built as part of this project) ----
set(SDL_SHARED OFF CACHE BOOL "" FORCE)   # link statically, no .dylib to ship
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# ---- Game sources ----
# Every .c under src/ is picked up automatically. CONFIGURE_DEPENDS makes
# CMake re-scan on each build, so a new file just needs a normal `cmake --build`.
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

add_executable(${PROJECT_NAME} ${GAME_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine   # reusable engine, knows nothing about the game
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game     # this game's scenes, state, and main
)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
