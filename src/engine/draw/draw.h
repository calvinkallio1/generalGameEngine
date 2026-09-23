#pragma once
/* 2D drawing primitives in logical coordinates. Blending is always on, so a Color
 * with alpha < 255 is translucent. Games use these instead of touching the renderer. */
#include "types.h"

/** Fill the whole frame with c. The engine already clears to EngineConfig.clear_color each frame before any scene renders; call this only to override it for one scene (a white flash, a different sky per level). Since it wipes everything, it must be the first draw call of the frame's bottom-most rendering scene.
 *  @see draw_rect, EngineConfig
 */
void draw_clear(Color c);                                        /* fill the whole frame */
/** Filled rectangle. Alpha blending is always on, so a translucent full-screen rect dims everything drawn before it: draw_rect(rect(0, 0, e->logical_w, e->logical_h), rgba(0, 0, 0, 160)) is the standard pause-menu backdrop. Placeholder art, health bars, UI panels and fade-to-black all reduce to this call.
 *  @see draw_rect_outline, rect, rgba
 */
void draw_rect(Rect r, Color c);                                 /* filled */
/** One-pixel outline of r, in logical pixels (so it scales up with the window). Handy for selection boxes and for seeing hitboxes; for the latter prefer debug_rect(), which only shows with the F3 overlay.
 *  @see draw_rect, debug_rect
 */
void draw_rect_outline(Rect r, Color c);
/** Line between two points in logical pixels, one pixel wide. Lasers, aim guides, grid lines, trajectory previews.
 *  @see draw_point, debug_line
 */
void draw_line(float x1, float y1, float x2, float y2, Color c);
/** Single logical pixel. Fine for stars and sparse particles; for many pixels a tiny sprite or draw_rect with w = h = 1 costs the same.
 *  @see draw_line, draw_rect
 */
void draw_point(float x, float y, Color c);
/** Copy a texture region (src, or NULL for the whole texture) into dst, scaling to fit. src and dst are in pixels of the texture and of the logical screen respectively; a dst larger than src scales up with nearest-neighbour filtering (pixel art stays crisp). For sprite sheets use sprite_draw(), which computes src from a frame index.
 *  @see draw_texture_ex, sprite_draw, assets_texture
 */
void draw_texture(Texture *t, const Rect *src, Rect dst);        /* src NULL = whole texture */
/** As draw_texture with rotation about dst's center (degrees, clockwise, 0 = as stored) and a Flip (FLIP_NONE, FLIP_H, FLIP_V, or FLIP_H | FLIP_V). Rotation is around the destination center, so a rotating sprite stays in place; to rotate about another point, offset dst accordingly.
 *  @see draw_texture, sprite_draw, Flip
 */
void draw_texture_ex(Texture *t, const Rect *src, Rect dst, double angle_degrees, Flip flip);
/** Text in the built-in 8x8 font, top-left at x,y. Always available, needs no font file and no EngineConfig.text, so it is the right tool for overlays, placeholder UI and anything drawn before fonts exist. Each character is 8 logical pixels wide and tall; the string is not wrapped or clipped.
 *  @see text_draw, debug_text
 */
void draw_debug_text(float x, float y, Color c, const char *s);  /* built-in 8x8 font */

/* engine-internal */
/** Engine-internal: remember the renderer and turn on alpha blending. Called once by engine_init(); games never call it.
 *  @see engine_init
 */
void          draw_init(SDL_Renderer *r);
/** Engine-internal: the renderer draw.h targets, for modules that must call SDL directly (text caching). Games never need it.
 *  @see draw_init
 */
SDL_Renderer *draw_renderer(void);
