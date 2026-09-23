#pragma once
#include <stdbool.h>
#include "types.h"
#include "mathx.h"

/** Build a Rect with its top-left corner at pos and the given size: the hitbox of an entity whose pos is its top-left. The entity template's hitbox helper is exactly this.
 *  @return the Rect.
 *  @see rect_centered, rect_overlaps, Rect
 */
static inline Rect rect_at(Vec2 pos, float w, float h)       { return (Rect){ pos.x, pos.y, w, h }; }
/** Build a Rect of the given size centered on c: the hitbox of an entity whose pos is its center, or a pickup radius as a box.
 *  @return the Rect.
 *  @see rect_at, rect_center
 */
static inline Rect rect_centered(Vec2 c, float w, float h)   { return (Rect){ c.x - w / 2, c.y - h / 2, w, h }; }
/** The center point of r. Aim at the middle of a target, spawn particles from the middle of a box.
 *  @return the center as a Vec2.
 *  @see rect_centered
 */
static inline Vec2      rect_center(Rect r)                  { return (Vec2){ r.x + r.w / 2, r.y + r.h / 2 }; }
/** Whether two rectangles overlap (share any area). The everyday collision test: bullet vs enemy, player vs pickup, entity vs trigger zone. Touching edges do not count. Test every pair with two loops; that is fast enough for hundreds of entities.
 *  @return true if a and b overlap.
 *  @see rect_hit_side, circle_overlaps, rect_contains
 */
static inline bool      rect_overlaps(Rect a, Rect b)   { return SDL_HasRectIntersectionFloat(&a, &b); }
/** Whether point p lies inside r. Mouse over a button: rect_contains(button, v2(in->mouse_x, in->mouse_y)); a point in a trigger; a tile center inside a room.
 *  @return true if p is inside r.
 *  @see rect_overlaps, mouse_pressed
 */
static inline bool      rect_contains(Rect r, Vec2 p)        { Point q = { p.x, p.y }; return SDL_PointInRectFloat(&q, &r); }

/** Whether two circles (centers a and b, radii ra and rb) overlap. Better than rectangles for round things and for a "pickup radius" around a point; no square root, so it is as cheap as rect_overlaps.
 *  @return true if the distance between centers is less than ra + rb.
 *  @see rect_overlaps, v2_dist
 */
static inline bool circle_overlaps(Vec2 a, float ra, Vec2 b, float rb) {
  float dx = a.x - b.x, dy = a.y - b.y, r = ra + rb;
  return dx * dx + dy * dy < r * r;
}

/** Which side of rectangle b rectangle a struck, as reported by rect_hit_side(): the side of b that a came from. HIT_TOP means a landed on top of b (a is above), HIT_BOTTOM that a hit b from below, HIT_LEFT that a is to the left of b, HIT_RIGHT to the right.
 *  @field HIT_LEFT a came from the left of b
 *  @field HIT_RIGHT a came from the right of b
 *  @field HIT_TOP a came from above b (landed on it)
 *  @field HIT_BOTTOM a came from below b (bumped its underside)
 *  @see rect_hit_side
 */
typedef enum { HIT_LEFT, HIT_RIGHT, HIT_TOP, HIT_BOTTOM } HitSide;
/** Given two overlapping rectangles, the side of b that a is shallowest on, i.e. the side a most likely arrived from. Decides what a collision means: a player HIT_TOP of an enemy stomps it, any other side takes damage; a ball HIT_LEFT or HIT_RIGHT of a paddle reverses its x velocity, HIT_TOP or HIT_BOTTOM its y. Only meaningful when rect_overlaps(a, b) is true. For sliding along solid tiles use tilemap_move() instead, which resolves each axis properly.
 *  @return the HitSide.
 *  @see rect_overlaps, HitSide, tilemap_move
 */
static inline HitSide rect_hit_side(Rect a, Rect b) {
  float ox = (a.x < b.x) ? (a.x + a.w - b.x) : (b.x + b.w - a.x);
  float oy = (a.y < b.y) ? (a.y + a.h - b.y) : (b.y + b.h - a.y);
  if (ox < oy) return a.x < b.x ? HIT_LEFT : HIT_RIGHT;
  return a.y < b.y ? HIT_TOP : HIT_BOTTOM;
}
