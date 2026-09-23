#pragma once
#include "types.h"
#include "mathx.h"

/** A window onto a world larger than the screen. pos is the world point shown at the center of the view; everything drawn through camera_to_screen() or camera_rect() is shifted and zoomed accordingly. Keep one in a scene static (presentation state, not GameState), initialize it in on_enter with camera_init(), move it in update with camera_follow(), and convert every world position in render. Screen shake is applied by writing offset each tick (from a Shake) rather than by moving pos, so follow and bounds are unaffected.
 *  @field pos world position at the center of the view
 *  @field zoom scale factor: 1 is one world pixel per logical pixel, 2 magnifies. Sprites drawn with sprite_draw_scaled(..., c->zoom, ...) match.
 *  @field view_w width of the view in logical pixels, normally e->logical_w
 *  @field view_h height of the view in logical pixels
 *  @field min top-left corner of the world bounds set by camera_set_bounds()
 *  @field max bottom-right corner of the bounds; min == max (the initial state) disables clamping
 *  @field offset added to pos at conversion time, for screen shake; not clamped
 *  @see camera_init, camera_follow, camera_to_screen, camera_visible, Shake, gge-camera
 */
typedef struct Camera {
  Vec2  pos;
  float zoom;
  int   view_w, view_h;   /* logical size */
  Vec2  min, max;         /* world bounds; min == max disables */
  Vec2  offset;           /* added at draw time, e.g. screen shake */
} Camera;

/** Reset a camera for a view of view_w x view_h logical pixels (normally e->logical_w, e->logical_h), positioned at the origin with zoom 1, no bounds and no offset. Call in on_enter, then set pos (or camera_follow with smoothing 0) so the first frame is not centered on 0,0.
 *  @see camera_follow, camera_to_screen, Camera
 */
void      camera_init(Camera *c, int view_w, int view_h);
/** Convert a world position to screen pixels for drawing: (world - pos - offset) * zoom + half the view. Use for every entity: Vec2 s = camera_to_screen(&cam, ent.pos); sprite_draw(&spr, f, s.x, s.y, 0, FLIP_NONE).
 *  @return the position in logical pixels.
 *  @see camera_to_world, camera_rect, sprite_draw
 */
Vec2      camera_to_screen(const Camera *c, Vec2 world);
/** Convert a screen position (e.g. the mouse, v2(in->mouse_x, in->mouse_y)) to world coordinates: the inverse of camera_to_screen. Divide by the tile size to get the tile under the cursor.
 *  @return the position in world units.
 *  @see camera_to_screen, Input
 */
Vec2      camera_to_world(const Camera *c, Vec2 screen);
/** Convert a world-space rectangle to screen space, scaling its size by zoom. Use to draw hitboxes or world-space panels with draw_rect.
 *  @return the Rect in logical pixels.
 *  @see camera_to_screen, draw_rect
 */
Rect camera_rect(const Camera *c, Rect world);
/** The world-space rectangle currently on screen (view size divided by zoom, centered on pos + offset); use to cull drawing: if (!rect_overlaps(camera_visible(&cam), ent_hitbox)) skip it. tilemap_draw() does this internally.
 *  @return the visible world Rect.
 *  @see tilemap_draw, rect_overlaps
 */
Rect camera_visible(const Camera *c);
/** Move the camera toward target. smoothing is a time constant in seconds: 0 snaps, 0.1-0.3 is a tight follow, 1.0 is lazy. Frame-rate independent. Call from update, after the target moved, so the frame drawn next sees the new position. When bounds are set the result is clamped so the view never leaves them; a world smaller than the view stays pinned to the bounds' top-left.
 *  @see camera_set_bounds, smooth_t, Camera
 */
void      camera_follow(Camera *c, Vec2 target, float smoothing, float dt);   /* smoothing 0 = snap */
/** Clamp the camera so it never shows outside the given world rectangle (the level edges): for a tilemap, camera_set_bounds(&cam, 0, 0, map.w * map.tile, map.h * map.tile). Applied by camera_follow(); setting pos directly bypasses it. Pass a zero-size rectangle to disable.
 *  @see camera_follow, Tilemap
 */
void      camera_set_bounds(Camera *c, float x, float y, float w, float h);
