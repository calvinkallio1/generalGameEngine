#include "sprite.h"
#include "draw.h"
#include <SDL3/SDL.h>

Sprite sprite_from(Texture *tex, int frame_w, int frame_h) {
  Sprite s = { tex, frame_w, frame_h, 1 };
  float w = 0, h = 0;
  if (tex && SDL_GetTextureSize(tex, &w, &h)) s.columns = (int)w / frame_w;
  if (s.columns < 1) s.columns = 1;
  return s;
}

void anim_set(AnimState *st, int anim_id) {
  if (st->anim == anim_id) return;
  st->anim = anim_id; st->frame = 0; st->elapsed = 0; st->done = false;
}

void anim_step(AnimState *st, const Anim *a, float dt) {
  if (st->done || a->count <= 1) return;
  st->elapsed += dt;
  while (st->elapsed >= a->frame_time) {
    st->elapsed -= a->frame_time;
    st->frame++;
    if (st->frame >= a->count) {
      if (a->loop) st->frame = 0;
      else { st->frame = a->count - 1; st->done = true; return; }
    }
  }
}

int anim_frame(const AnimState *st, const Anim *a) { return a->first + st->frame; }

void sprite_draw_scaled(const Sprite *s, int frame, float x, float y, float scale, double angle, Flip flip) {
  if (!s->tex) return;
  Rect src = { (float)((frame % s->columns) * s->frame_w), (float)((frame / s->columns) * s->frame_h),
                    (float)s->frame_w, (float)s->frame_h };
  Rect dst = { x, y, s->frame_w * scale, s->frame_h * scale };
  draw_texture_ex(s->tex, &src, dst, angle, flip);
}

void sprite_draw(const Sprite *s, int frame, float x, float y, double angle, Flip flip) {
  sprite_draw_scaled(s, frame, x, y, 1.0f, angle, flip);
}

void sprite_draw_tinted(const Sprite *s, int frame, float x, float y, Color tint, Flip flip) {
  SDL_SetTextureColorMod(s->tex, tint.r, tint.g, tint.b);
  SDL_SetTextureAlphaMod(s->tex, tint.a);
  sprite_draw(s, frame, x, y, 0, flip);
  SDL_SetTextureColorMod(s->tex, 255, 255, 255);
  SDL_SetTextureAlphaMod(s->tex, 255);
}
