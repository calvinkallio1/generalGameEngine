#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "types.h"
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
bool    tilemap_box_hits(const Tilemap *m, const Rect *box);
void    tilemap_draw(const Tilemap *m, const Camera *c);
void    tilemap_move(const Tilemap *m, Rect *box, Vec2 *vel, float dt, bool *on_ground);

/* Load a Tiled "CSV" layer export (File > Export As > CSV writes one file per layer).
   Allocates m->tiles; call tilemap_free when done. Tiled writes global ids (0 = empty,
   first tile = 1); they are stored as id - 1 so tileset frame N is the tile Tiled shows
   as N. Frame 0 of the sheet is therefore the "empty" tile and is never drawn.
   solid is your own 256-entry table (CSV carries no tile properties).
   Returns false (and logs) on any error. */
bool    tilemap_load_csv(Tilemap *m, const char *relative_path, int tile_size,
                         Texture *tileset, const bool *solid);
void    tilemap_free(Tilemap *m);
