#pragma once
#include <SDL3/SDL.h>
#include <stdbool.h>
#include "mathx.h"

typedef struct Sprite { SDL_Texture *tex; int frame_w, frame_h; int columns; } Sprite;
typedef struct Anim   { int first, count; float frame_time; bool loop; } Anim;
typedef struct AnimState { int anim; int frame; float elapsed; bool done; } AnimState;

Sprite sprite_from(SDL_Texture *tex, int frame_w, int frame_h);
void   anim_set(AnimState *st, int anim_id);                /* no-op if already playing */
void   anim_step(AnimState *st, const Anim *a, float dt);
int    anim_frame(const AnimState *st, const Anim *a);      /* absolute sheet frame */

void sprite_draw(SDL_Renderer *r, const Sprite *s, int frame, float x, float y, double angle, SDL_FlipMode flip);
void sprite_draw_scaled(SDL_Renderer *r, const Sprite *s, int frame, float x, float y, float scale, double angle, SDL_FlipMode flip);
