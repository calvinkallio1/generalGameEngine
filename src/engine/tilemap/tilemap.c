#include "tilemap.h"
#include "assets.h"
#include "debug.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t tilemap_at(const Tilemap *m, int tx, int ty) {
  if (tx < 0 || ty < 0 || tx >= m->w || ty >= m->h) return 0;
  return m->tiles[ty * m->w + tx];
}

bool tilemap_solid_at(const Tilemap *m, float wx, float wy) {
  return m->solid[tilemap_at(m, (int)floorf(wx / m->tile), (int)floorf(wy / m->tile))];
}

bool tilemap_box_hits(const Tilemap *m, const Rect *b) {
  int x0 = (int)floorf(b->x / m->tile),                     y0 = (int)floorf(b->y / m->tile);
  int x1 = (int)floorf((b->x + b->w - 0.001f) / m->tile),   y1 = (int)floorf((b->y + b->h - 0.001f) / m->tile);
  for (int ty = y0; ty <= y1; ++ty)
    for (int tx = x0; tx <= x1; ++tx)
      if (m->solid[tilemap_at(m, tx, ty)]) return true;
  return false;
}

static bool color_eq(Color a, Color b) { return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a; }

void tilemap_draw(const Tilemap *m, const Camera *c) {
  if (!m->tileset.tex) return;
  Rect vis = camera_visible(c);
  int x0 = (int)floorf(vis.x / m->tile) - 1, x1 = (int)ceilf((vis.x + vis.w) / m->tile) + 1;
  int y0 = (int)floorf(vis.y / m->tile) - 1, y1 = (int)ceilf((vis.y + vis.h) / m->tile) + 1;
  Color cur = COLOR_WHITE;                       /* the tileset texture's current modulation */
  for (int ty = y0; ty <= y1; ++ty) {
    for (int tx = x0; tx <= x1; ++tx) {
      uint8_t id = tilemap_at(m, tx, ty);
      if (id == 0) continue;                     /* also skips cells outside the map, so the tint index is safe */
      Color want = COLOR_WHITE;
      if (m->tints) {
        Color t = m->tints[ty * m->w + tx];
        if (t.a) want = t;                       /* alpha 0 = "untinted" for this cell */
      }
      if (!color_eq(want, cur)) {                /* set the modulation only when it changes */
        SDL_SetTextureColorMod(m->tileset.tex, want.r, want.g, want.b);
        SDL_SetTextureAlphaMod(m->tileset.tex, want.a);
        cur = want;
      }
      Vec2 s = camera_to_screen(c, (Vec2){ (float)(tx * m->tile), (float)(ty * m->tile) });
      sprite_draw_scaled(&m->tileset, id, s.x, s.y, c->zoom, 0, FLIP_NONE);
    }
  }
  if (!color_eq(cur, COLOR_WHITE)) {             /* leave the shared texture neutral for other draws */
    SDL_SetTextureColorMod(m->tileset.tex, 255, 255, 255);
    SDL_SetTextureAlphaMod(m->tileset.tex, 255);
  }
}

void tilemap_move(const Tilemap *m, Rect *box, Vec2 *vel, float dt, bool *on_ground) {
  box->x += vel->x * dt;
  if (tilemap_box_hits(m, box)) {
    if (vel->x > 0) box->x = floorf((box->x + box->w) / m->tile) * m->tile - box->w;
    else            box->x = ceilf(box->x / m->tile) * m->tile;
    vel->x = 0;
  }
  if (on_ground) *on_ground = false;
  box->y += vel->y * dt;
  if (tilemap_box_hits(m, box)) {
    if (vel->y > 0) { box->y = floorf((box->y + box->h) / m->tile) * m->tile - box->h; if (on_ground) *on_ground = true; }
    else            { box->y = ceilf(box->y / m->tile) * m->tile; }
    vel->y = 0;
  }
}

bool tilemap_load_csv(Tilemap *m, const char *relative_path, int tile_size,
                      Texture *tileset, const bool *solid) {
  char path[1024];
  FILE *f = fopen(assets_path(relative_path, path, sizeof path), "rb");
  if (!f) { LOG("tilemap: cannot open %s", path); return false; }

  /* pass 1: measure. width = commas on the first line + 1, height = non-empty lines */
  int w = 0, h = 0, c, in_row = 0;
  while ((c = fgetc(f)) != EOF) {
    if (c == ',' && h == 0) ++w;
    if (c != '\n' && c != '\r' && c != ' ') in_row = 1;
    if (c == '\n' && in_row) { ++h; in_row = 0; }
  }
  if (in_row) ++h;                       /* last line without a trailing newline */
  ++w;
  if (h <= 0) { LOG("tilemap: %s is empty", path); fclose(f); return false; }

  /* pass 2: read */
  uint8_t *tiles = calloc((size_t)w * (size_t)h, 1);
  if (!tiles) { fclose(f); return false; }
  rewind(f);
  for (int i = 0; i < w * h; ++i) {
    long gid;
    if (fscanf(f, " %ld", &gid) != 1) { LOG("tilemap: bad cell %d in %s", i, path); break; }
    tiles[i] = gid > 0 ? (uint8_t)(gid - 1) : 0;
    if (fscanf(f, " ,") < 0) break;      /* eat the separator if present */
  }
  fclose(f);

  m->w = w; m->h = h; m->tile = tile_size;
  m->tiles = tiles;
  m->solid = solid;
  m->tileset = sprite_from(tileset, tile_size, tile_size);
  return true;
}

void tilemap_free(Tilemap *m) {
  free((void *)m->tiles);               /* const on the struct guards static arrays; ours is malloc'd */
  m->tiles = NULL;
  m->w = m->h = 0;
}
