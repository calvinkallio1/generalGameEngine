#include "assets.h"
#include <stdio.h>
#include <string.h>
#ifdef ENGINE_WITH_IMAGE
#include <SDL3_image/SDL_image.h>
#endif

#define ASSETS_MAX 128
typedef struct { char path[128]; SDL_Texture *tex; } Entry;

static SDL_Renderer *renderer;
static Entry table[ASSETS_MAX];
static int   count;
static char  base[512];

void assets_init(SDL_Renderer *r) {
  renderer = r;
  const char *exe_dir = SDL_GetBasePath();   /* owned by SDL, do not free */
  snprintf(base, sizeof base, "%sassets/", exe_dir ? exe_dir : "./");
}

const char *assets_path(const char *rel, char *buf, size_t n) {
  snprintf(buf, n, "%s%s", base, rel);
  return buf;
}

static SDL_Texture *load(const char *full) {
#ifdef ENGINE_WITH_IMAGE
  SDL_Surface *surf = IMG_Load(full);
#else
  SDL_Surface *surf = SDL_LoadBMP(full);
#endif
  if (!surf) { SDL_Log("assets: %s: %s", full, SDL_GetError()); return NULL; }
  SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
  SDL_DestroySurface(surf);
  if (!tex) { SDL_Log("assets: %s: %s", full, SDL_GetError()); return NULL; }
  SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
  return tex;
}

SDL_Texture *assets_texture(const char *rel) {
  for (int i = 0; i < count; ++i)
    if (strcmp(table[i].path, rel) == 0) return table[i].tex;
  if (count >= ASSETS_MAX) { SDL_Log("assets: table full"); return NULL; }
  char full[640];
  SDL_Texture *tex = load(assets_path(rel, full, sizeof full));
  if (!tex) return NULL;
  snprintf(table[count].path, sizeof table[count].path, "%s", rel);
  table[count].tex = tex;
  return table[count++].tex;
}

void assets_unload_all(void) {
  for (int i = 0; i < count; ++i) SDL_DestroyTexture(table[i].tex);
  count = 0;
}
