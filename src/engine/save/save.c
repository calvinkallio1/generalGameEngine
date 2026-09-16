#include "save.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#define SAVE_MAGIC 0x45564153u
typedef struct { uint32_t magic, version, size; } SaveHeader;

void save_init(Save *s, const char *org, const char *app) {
  char *p = (org && app) ? SDL_GetPrefPath(org, app) : NULL;   /* creates the directory */
  snprintf(s->dir, sizeof s->dir, "%s", p ? p : "./");
  SDL_free(p);
}

static void path_for(const Save *s, const char *slot, char *out, size_t n) {
  snprintf(out, n, "%s%s.sav", s->dir, slot);
}

bool save_exists(const Save *s, const char *slot) {
  char path[640]; path_for(s, slot, path, sizeof path);
  return SDL_GetPathInfo(path, NULL);
}

bool save_delete(const Save *s, const char *slot) {
  char path[640]; path_for(s, slot, path, sizeof path);
  return SDL_RemovePath(path);
}

bool save_write(const Save *s, const char *slot, const void *data, size_t size, uint32_t version) {
  char path[640]; path_for(s, slot, path, sizeof path);
  SaveHeader h = { SAVE_MAGIC, version, (uint32_t)size };
  SDL_IOStream *f = SDL_IOFromFile(path, "wb");
  if (!f) { SDL_Log("save: %s: %s", path, SDL_GetError()); return false; }
  bool ok = SDL_WriteIO(f, &h, sizeof h) == sizeof h && SDL_WriteIO(f, data, size) == size;
  SDL_CloseIO(f);
  return ok;
}

bool save_load(const Save *s, const char *slot, void *out, size_t size, uint32_t version) {
  char path[640]; path_for(s, slot, path, sizeof path);
  size_t n = 0;
  void *blob = SDL_LoadFile(path, &n);
  if (!blob) return false;
  bool ok = false;
  if (n == sizeof(SaveHeader) + size) {
    SaveHeader h; memcpy(&h, blob, sizeof h);
    if (h.magic == SAVE_MAGIC && h.version == version && h.size == size) {
      memcpy(out, (unsigned char *)blob + sizeof h, size);
      ok = true;
    } else SDL_Log("save: %s rejected (version %u, expected %u)", path, h.version, version);
  }
  SDL_free(blob);
  return ok;
}
