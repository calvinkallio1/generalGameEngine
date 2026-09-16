cmake_minimum_required(VERSION 3.16)
project(game_engine LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # clangd / editor support

# ---- SDL3 (vendored submodule, built as part of this project) ----
set(SDL_SHARED OFF CACHE BOOL "" FORCE)   # link statically, no .dylib to ship
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# ---- Engine library: src/engine/ ----
# Every .c under src/engine/ is part of the library. CONFIGURE_DEPENDS makes
# CMake re-scan on each build, so a new file just needs a normal `cmake --build`.
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/*.c
)
add_library(engine STATIC ${ENGINE_SOURCES})
# PUBLIC: anything that links `engine` also gets its headers and SDL3.
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/engine)
target_link_libraries(engine PUBLIC SDL3::SDL3)
target_compile_options(engine PRIVATE -Wall -Wextra)

# ---- Game executable: src/game/ ----
# This repo ships src/game/ empty. Drop a main.c and your scenes in there (or
# use this repo as a template for a game) and the executable appears on the
# next build. The engine library builds either way.
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game/*.c
)
if(GAME_SOURCES)
    add_executable(${PROJECT_NAME} ${GAME_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/game)
    target_link_libraries(${PROJECT_NAME} PRIVATE engine)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()
