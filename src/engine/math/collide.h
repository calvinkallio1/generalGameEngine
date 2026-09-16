#pragma once
#include <SDL3/SDL.h>
#include <stdbool.h>
#include "mathx.h"

static inline SDL_FRect rect_at(Vec2 pos, float w, float h)       { return (SDL_FRect){ pos.x, pos.y, w, h }; }
static inline SDL_FRect rect_centered(Vec2 c, float w, float h)   { return (SDL_FRect){ c.x - w / 2, c.y - h / 2, w, h }; }
static inline Vec2      rect_center(SDL_FRect r)                  { return (Vec2){ r.x + r.w / 2, r.y + r.h / 2 }; }
static inline bool      rect_overlaps(SDL_FRect a, SDL_FRect b)   { return SDL_HasRectIntersectionFloat(&a, &b); }
static inline bool      rect_contains(SDL_FRect r, Vec2 p)        { SDL_FPoint q = { p.x, p.y }; return SDL_PointInRectFloat(&q, &r); }

static inline bool circle_overlaps(Vec2 a, float ra, Vec2 b, float rb) {
  float dx = a.x - b.x, dy = a.y - b.y, r = ra + rb;
  return dx * dx + dy * dy < r * r;
}

typedef enum { HIT_LEFT, HIT_RIGHT, HIT_TOP, HIT_BOTTOM } HitSide;
static inline HitSide rect_hit_side(SDL_FRect a, SDL_FRect b) {
  float ox = (a.x < b.x) ? (a.x + a.w - b.x) : (b.x + b.w - a.x);
  float oy = (a.y < b.y) ? (a.y + a.h - b.y) : (b.y + b.h - a.y);
  if (ox < oy) return a.x < b.x ? HIT_LEFT : HIT_RIGHT;
  return a.y < b.y ? HIT_TOP : HIT_BOTTOM;
}
