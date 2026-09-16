#pragma once
/* 2D drawing primitives in logical coordinates. Blending is always on, so a Color
 * with alpha < 255 is translucent. Games use these instead of touching the renderer. */
#include "types.h"

void draw_clear(Color c);                                        /* fill the whole frame */
void draw_rect(Rect r, Color c);                                 /* filled */
void draw_rect_outline(Rect r, Color c);
void draw_line(float x1, float y1, float x2, float y2, Color c);
void draw_point(float x, float y, Color c);
void draw_texture(Texture *t, const Rect *src, Rect dst);        /* src NULL = whole texture */
void draw_texture_ex(Texture *t, const Rect *src, Rect dst, double angle_degrees, Flip flip);
void draw_debug_text(float x, float y, Color c, const char *s);  /* built-in 8x8 font */

/* engine-internal */
void          draw_init(SDL_Renderer *r);
SDL_Renderer *draw_renderer(void);
