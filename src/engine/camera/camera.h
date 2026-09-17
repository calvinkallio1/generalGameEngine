#pragma once
#include "types.h"
#include "mathx.h"

typedef struct Camera {
  Vec2  pos;
  float zoom;
  int   view_w, view_h;   /* logical size */
  Vec2  min, max;         /* world bounds; min == max disables */
  Vec2  offset;           /* added at draw time, e.g. screen shake */
} Camera;

/** Reset a camera for a view of view_w x view_h logical pixels, positioned at the origin with zoom 1.
 *  @see camera_follow, camera_to_screen
 */
void      camera_init(Camera *c, int view_w, int view_h);
/** Convert a world position to screen pixels for drawing.
 *  @see camera_to_world, camera_rect
 */
Vec2      camera_to_screen(const Camera *c, Vec2 world);
/** Convert a screen position (e.g. the mouse) to world coordinates.
 *  @see camera_to_screen
 */
Vec2      camera_to_world(const Camera *c, Vec2 screen);
/** Convert a world-space rectangle to screen space.
 *  @see camera_to_screen
 */
Rect camera_rect(const Camera *c, Rect world);
/** The world-space rectangle currently on screen; use to cull drawing.
 *  @see tilemap_draw
 */
Rect camera_visible(const Camera *c);
/** Move the camera toward target. smoothing is a time constant in seconds: 0 snaps, 0.1-0.3 is a tight follow, 1.0 is lazy. Call from update.
 *  @see camera_set_bounds
 */
void      camera_follow(Camera *c, Vec2 target, float smoothing, float dt);   /* smoothing 0 = snap */
/** Clamp the camera so it never shows outside the given world rectangle (the level edges).
 *  @see camera_follow
 */
void      camera_set_bounds(Camera *c, float x, float y, float w, float h);
