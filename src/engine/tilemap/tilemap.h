#pragma once
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "sprite.h"
#include "camera.h"

typedef struct Tilemap {
  int w, h;                 /* in tiles */
  int tile;                 /* tile size in pixels (square) */
  const uint8_t *tiles;     /* w*h ids, row-major; 0 = empty */
  const bool    *solid;     /* indexed by tile id (256 entries) */
  Sprite tileset;           /* frame index == tile id */
} Tilemap;

uint8_t tilemap_at(const Tilemap *m, int tx, int ty);             /* 0 outside */
bool    tilemap_solid_at(const Tilemap *m, float wx, float wy);
bool    tilemap_box_hits(const Tilemap *m, const SDL_FRect *box);
void    tilemap_draw(SDL_Renderer *r, const Tilemap *m, const Camera *c);
void    tilemap_move(const Tilemap *m, SDL_FRect *box, Vec2 *vel, float dt, bool *on_ground);
