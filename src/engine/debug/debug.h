#pragma once
#include <SDL3/SDL.h>
#include "types.h"

/** printf-style message to the terminal (stderr): LOG("loaded level %d", n). The engine's own messages (missing assets, save failures) use it too, so run the game from a terminal to see them. No newline needed.
 *  @see WARN, ASSERT, debug_text
 */
#define LOG(...)   SDL_Log(__VA_ARGS__)
/** As LOG, at warning priority. Same output on most platforms; use it for things that are wrong but survivable (an entity array is full, a save was rejected) so they stand out when filtering logs.
 *  @see LOG, ASSERT
 */
#define WARN(...)  SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
/** Stop the program (with a message naming the file and line) if condition c is false, in Debug builds; compiled out in Release builds, so c must have no side effects. Guard invariants: ASSERT(g->count <= MAX_ENTITIES), ASSERT(index >= 0).
 *  @see LOG, WARN
 */
#define ASSERT(c)  SDL_assert(c)

/** Queue a rectangle outline to be drawn on top of everything this frame, but only while the F3 debug overlay is on. Hitboxes, trigger zones, camera bounds: call it from update or render wherever the data is handy, and the shapes appear over the finished frame. The queue holds 256 shapes per frame; extra calls are dropped. Coordinates are logical screen pixels, so convert world rectangles with camera_rect() first. Costs nothing when the overlay is off besides the queue write.
 *  @see debug_line, debug_text, draw_rect_outline, camera_rect
 */
void debug_rect(Rect r, Color c);
/** Queue a line for the F3 overlay, like debug_rect(). Velocity vectors, aim directions, pathfinding routes, raycasts.
 *  @see debug_rect, debug_text
 */
void debug_line(float x1, float y1, float x2, float y2, Color c);
/** Queue a printf-formatted string in yellow 8x8 text for the F3 overlay at x,y (logical pixels): debug_text(4, 20, "hp %d  state %d", p->hp, p->state). Up to 32 strings of 127 characters per frame. The engine writes its own status line at 4,4; start yours at y = 20 or so. Use LOG() for things that should go to the terminal instead.
 *  @see debug_rect, draw_debug_text
 */
void debug_text(float x, float y, const char *fmt, ...);

/* engine-internal: draws (if `draw`) and clears the queue every frame */
/** Engine-internal: draw the queued shapes and text if draw is true, then empty the queues. Called once per frame by engine_run() after the scenes render; games never call it.
 *  @see engine_run
 */
void debug_flush(bool draw);
