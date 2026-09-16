#pragma once
#include <stdbool.h>
#include "types.h"
#include "mathx.h"

typedef struct Sprite { Texture *tex; int frame_w, frame_h; int columns; } Sprite;
typedef struct Anim   { int first, count; float frame_time; bool loop; } Anim;
typedef struct AnimState { int anim; int frame; float elapsed; bool done; } AnimState;

Sprite sprite_from(Texture *tex, int frame_w, int frame_h);
void   anim_set(AnimState *st, int anim_id);                /* no-op if already playing */
void   anim_step(AnimState *st, const Anim *a, float dt);
int    anim_frame(const AnimState *st, const Anim *a);      /* absolute sheet frame */

void sprite_draw(const Sprite *s, int frame, float x, float y, double angle, Flip flip);
void sprite_draw_scaled(const Sprite *s, int frame, float x, float y, float scale, double angle, Flip flip);

/* Draw with the texture's colors multiplied by tint (white sheet x tint = tint; alpha
   fades). Used for palette-free recoloring: layered white silhouettes tinted per entity.
   The texture's modulation is restored to neutral afterward, so other draws are unaffected. */
void sprite_draw_tinted(const Sprite *s, int frame, float x, float y, Color tint, Flip flip);
