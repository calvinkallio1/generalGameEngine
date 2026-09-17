#pragma once
/* 2D drawing primitives in logical coordinates. Blending is always on, so a Color
 * with alpha < 255 is translucent. Games use these instead of touching the renderer. */
#include "types.h"

/** Fill the whole frame with c. The engine already clears to EngineConfig.clear_color each frame; call this only to override.
 *  @see draw_rect
 */
void draw_clear(Color c);                                        /* fill the whole frame */
/** Filled rectangle. Alpha blending is always on, so a translucent full-screen rect dims everything drawn before it.
 *  @see draw_rect_outline
 */
void draw_rect(Rect r, Color c);                                 /* filled */
/** One-pixel outline of r.
 *  @see draw_rect
 */
void draw_rect_outline(Rect r, Color c);
/** Line between two points in logical pixels.
 *  @see draw_point
 */
void draw_line(float x1, float y1, float x2, float y2, Color c);
/** Single pixel.
 *  @see draw_line
 */
void draw_point(float x, float y, Color c);
/** Copy a texture region (src, or NULL for the whole texture) into dst, scaling to fit.
 *  @see draw_texture_ex, sprite_draw
 */
void draw_texture(Texture *t, const Rect *src, Rect dst);        /* src NULL = whole texture */
/** As draw_texture with rotation about dst's center (degrees, clockwise) and a Flip (FLIP_NONE, FLIP_H, FLIP_V, FLIP_HV).
 *  @see draw_texture
 */
void draw_texture_ex(Texture *t, const Rect *src, Rect dst, double angle_degrees, Flip flip);
/** Text in the built-in 8x8 font. Always available; ideal for overlays and placeholder UI.
 *  @see text_draw
 */
void draw_debug_text(float x, float y, Color c, const char *s);  /* built-in 8x8 font */

/* engine-internal */
void          draw_init(SDL_Renderer *r);
SDL_Renderer *draw_renderer(void);
