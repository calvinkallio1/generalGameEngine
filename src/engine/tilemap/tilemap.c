#include "tilemap.h"
#include <math.h>

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

void tilemap_draw(const Tilemap *m, const Camera *c) {
  Rect vis = camera_visible(c);
  int x0 = (int)floorf(vis.x / m->tile) - 1, x1 = (int)ceilf((vis.x + vis.w) / m->tile) + 1;
  int y0 = (int)floorf(vis.y / m->tile) - 1, y1 = (int)ceilf((vis.y + vis.h) / m->tile) + 1;
  for (int ty = y0; ty <= y1; ++ty) {
    for (int tx = x0; tx <= x1; ++tx) {
      uint8_t id = tilemap_at(m, tx, ty);
      if (id == 0) continue;
      Vec2 s = camera_to_screen(c, (Vec2){ (float)(tx * m->tile), (float)(ty * m->tile) });
      sprite_draw_scaled(&m->tileset, id, s.x, s.y, c->zoom, 0, FLIP_NONE);
    }
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
