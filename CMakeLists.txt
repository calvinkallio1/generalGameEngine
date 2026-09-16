cmake_minimum_required(VERSION 3.21)
project(game_engine LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # clangd / editor support

# Optional dependencies. A game turns these on with set(... ON CACHE BOOL "" FORCE)
# before add_subdirectory(); the matching submodule must be checked out under vendored/.
option(ENGINE_WITH_IMAGE "PNG/JPG loading via SDL_image (vendored/SDL_image)" OFF)
option(ENGINE_WITH_TTF   "TrueType fonts via SDL_ttf (vendored/SDL_ttf)"    OFF)

# Everything vendored builds static: SDL uses its own SDL_SHARED/SDL_STATIC switches,
# SDL_image and SDL_ttf use the standard BUILD_SHARED_LIBS. A shared SDL_image cannot
# link against a static SDL, so all three must agree.
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# ---- SDL3 (vendored submodule, built as part of this project) ----
set(SDL_SHARED OFF CACHE BOOL "" FORCE)   # link statically, no .dylib to ship
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# ---- Engine library: one folder per module under src/engine/ ----
# Every .c anywhere under src/engine/ is part of the library. CONFIGURE_DEPENDS makes
# CMake re-scan on each build, so a new file or folder just needs `cmake --build`.
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/*.c
)
add_library(engine STATIC ${ENGINE_SOURCES})

# Every module folder is a PUBLIC include dir, so any header is included by bare
# name (#include "assets.h") from inside the engine and from a game that links it.
file(GLOB ENGINE_MODULE_DIRS LIST_DIRECTORIES true CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/*
)
foreach(dir ${ENGINE_MODULE_DIRS})
    if(IS_DIRECTORY ${dir})
        target_include_directories(engine PUBLIC ${dir})
    endif()
endforeach()

target_link_libraries(engine PUBLIC SDL3::SDL3)
target_compile_options(engine PRIVATE -Wall -Wextra)
if(NOT APPLE AND NOT WIN32)
    target_link_libraries(engine PUBLIC m)   # sqrtf/expf/powf on Linux
endif()

if(ENGINE_WITH_IMAGE)
    set(SDLIMAGE_VENDORED OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)   # trim formats that need extra libraries
    set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_TIF  OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_DEPS_SHARED OFF CACHE BOOL "" FORCE)   # no dlopen of image libraries at runtime
    set(SDLIMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)
    target_link_libraries(engine PUBLIC SDL3_image::SDL3_image)
    target_compile_definitions(engine PUBLIC ENGINE_WITH_IMAGE)
endif()

if(ENGINE_WITH_TTF)
    set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)    # builds FreeType from SDL_ttf's own submodule
    set(SDLTTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
    set(SDLTTF_SAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(vendored/SDL_ttf EXCLUDE_FROM_ALL)
    target_link_libraries(engine PUBLIC SDL3_ttf::SDL3_ttf)
    target_compile_definitions(engine PUBLIC ENGINE_WITH_TTF)
endif()

# ---- Game executable: src/game/ ----
# Ships empty. Drop a main.c (and scenes) in and the executable appears on the next build.
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game/*.c
)
if(GAME_SOURCES)
    add_executable(${PROJECT_NAME} ${GAME_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/game)
    target_link_libraries(${PROJECT_NAME} PRIVATE engine)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)
    endif()
endif()
