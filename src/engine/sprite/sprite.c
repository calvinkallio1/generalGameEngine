#include "sprite.h"

Sprite sprite_from(SDL_Texture *tex, int frame_w, int frame_h) {
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

void sprite_draw_scaled(SDL_Renderer *r, const Sprite *s, int frame, float x, float y, float scale, double angle, SDL_FlipMode flip) {
  if (!s->tex) return;
  SDL_FRect src = { (float)((frame % s->columns) * s->frame_w), (float)((frame / s->columns) * s->frame_h),
                    (float)s->frame_w, (float)s->frame_h };
  SDL_FRect dst = { x, y, s->frame_w * scale, s->frame_h * scale };
  SDL_RenderTextureRotated(r, s->tex, &src, &dst, angle, NULL, flip);
}

void sprite_draw(SDL_Renderer *r, const Sprite *s, int frame, float x, float y, double angle, SDL_FlipMode flip) {
  sprite_draw_scaled(r, s, frame, x, y, 1.0f, angle, flip);
}
