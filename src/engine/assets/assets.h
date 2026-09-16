#pragma once
#include <SDL3/SDL.h>
#include "types.h"

Texture     *assets_texture(const char *relative_path);                     /* "sprites/player.bmp" */
const char  *assets_path(const char *relative_path, char *buf, size_t n);   /* absolute path helper */

/* engine-internal */
void assets_init(SDL_Renderer *r);
void assets_unload_all(void);
