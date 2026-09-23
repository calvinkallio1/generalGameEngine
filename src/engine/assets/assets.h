#pragma once
#include <SDL3/SDL.h>
#include "types.h"

/** Load an image from assets/ (path relative to that folder, e.g. "sprites/player.png") as a GPU texture, or return the one already loaded for that path. The cache is keyed by the exact string, so "sprites/a.png" and "./sprites/a.png" load twice. PNG, JPG and other formats need the engine built with ENGINE_WITH_IMAGE; without it only .bmp loads. Textures use nearest-neighbour scaling (crisp pixel art). Never free the result; every texture lives until engine_shutdown(). Call it freely in on_enter, or even every frame: a cache hit is a string compare. Up to 128 distinct textures.
 *  @return the Texture, or NULL (logged) if the file is missing, unreadable, or the cache is full. Every draw call accepts NULL and draws nothing, so a missing image shows as an absence rather than a crash.
 *  @see assets_path, draw_texture, sprite_from, Texture, gge-assets
 */
Texture     *assets_texture(const char *relative_path);                     /* "sprites/player.bmp" */
/** Build the absolute path of a file under assets/ into buf (n bytes) and return buf. assets/ is located next to the executable, where CMake copies the project's assets/ folder after each build. Use it to open your own data files (level text, dialogue scripts, CSV tables) with fopen: char p[1024]; FILE *f = fopen(assets_path("levels/1.txt", p, sizeof p), "rb");.
 *  @return buf, for use directly inside a call.
 *  @see assets_texture, tilemap_load_csv, gge-assets
 */
const char  *assets_path(const char *relative_path, char *buf, size_t n);   /* absolute path helper */

/* engine-internal */
/** Engine-internal: remember the renderer and locate the assets/ folder next to the executable. Called by engine_init(); games never call it.
 *  @see engine_init
 */
void assets_init(SDL_Renderer *r);
/** Engine-internal: destroy every cached texture. Called by engine_shutdown(); games never call it.
 *  @see engine_shutdown
 */
void assets_unload_all(void);
