#pragma once
#include <stdbool.h>
#include "types.h"
#include "mathx.h"

/** A texture sliced into a grid of equal frames. Frames are numbered left to right, top to bottom, starting at 0: frame = column + row * columns. A Sprite is a small value type: copy it freely, keep one per sheet in a scene static, and never free it (the Texture belongs to the asset cache). Build it with sprite_from().
 *  @field tex the sheet; NULL draws nothing
 *  @field frame_w width of one frame in pixels
 *  @field frame_h height of one frame in pixels
 *  @field columns frames per row, computed from the texture width by sprite_from()
 *  @see sprite_from, sprite_draw, Anim, gge-sprite
 */
typedef struct Sprite { Texture *tex; int frame_w, frame_h; int columns; } Sprite;
/** One named animation: a contiguous run of frames on a sheet and how fast to play them. Define them as static const data, one per action, usually in an array indexed by an enum: static const Anim cat_anims[] = { [CAT_IDLE] = { 0, 4, 0.25f, true }, [CAT_RUN] = { 4, 6, 0.08f, true } };. An Anim holds no playback position; that is AnimState.
 *  @field first sheet index of the first frame
 *  @field count number of frames; 1 is a still
 *  @field frame_time seconds each frame is shown
 *  @field loop true to wrap around forever, false to stop on the last frame and set AnimState.done
 *  @see AnimState, anim_step, anim_frame, Sprite
 */
typedef struct Anim   { int first, count; float frame_time; bool loop; } Anim;
/** Playback position of one animated thing. Pointer-free, so it can live inside an entity in GameState and survive a save. Drive it with anim_set() to choose an animation and anim_step() every tick; read the sheet frame with anim_frame().
 *  @field anim id of the current animation (your index into an Anim table)
 *  @field frame frame within the animation, 0..count-1
 *  @field elapsed seconds accumulated toward the next frame
 *  @field done true once a non-looping animation reached its last frame
 *  @see anim_set, anim_step, anim_frame, Anim
 */
typedef struct AnimState { int anim; int frame; float elapsed; bool done; } AnimState;

/** Slice a texture into frame_w x frame_h cells and return the Sprite. columns is derived from the texture width, so a 128-pixel-wide sheet of 16-pixel frames has 8 columns and frame 9 is row 1, column 1. Rows are implied by the height. A NULL texture yields a Sprite that draws nothing. For a single image, pass its full width and height and always draw frame 0.
 *  @return the Sprite, by value.
 *  @see sprite_draw, assets_texture, Sprite
 */
Sprite sprite_from(Texture *tex, int frame_w, int frame_h);
/** Switch st to animation anim_id, restarting at its first frame with done cleared. A no-op if that animation is already playing, so it is safe to call every tick from the code that decides what the entity is doing (if (moving) anim_set(&st, RUN); else anim_set(&st, IDLE);) without the animation constantly restarting.
 *  @see anim_step, anim_frame, AnimState
 */
void   anim_set(AnimState *st, int anim_id);                /* no-op if already playing */
/** Advance the animation by dt seconds using the timing in a (the Anim that st->anim refers to). Call once per update tick. Several frames can advance in one call after a long dt. Looping animations wrap; others stop on the last frame and set st->done, after which the call does nothing until anim_set() restarts them. A count of 1 never advances.
 *  @see anim_set, anim_frame, Anim
 */
void   anim_step(AnimState *st, const Anim *a, float dt);
/** The absolute sheet frame to draw right now: a->first + st->frame. Pass the result to sprite_draw().
 *  @return the frame index on the sheet.
 *  @see anim_step, sprite_draw
 */
int    anim_frame(const AnimState *st, const Anim *a);      /* absolute sheet frame */

/** Draw one frame of s with its top-left corner at x,y (logical pixels), rotated by angle degrees clockwise about the frame's center, mirrored by flip. Frames are drawn at their natural size. Under a camera, convert the position first with camera_to_screen(). Draw order is call order: draw the ground before the player and the player before the HUD.
 *  @see sprite_draw_scaled, sprite_draw_tinted, anim_frame, camera_to_screen
 */
void sprite_draw(const Sprite *s, int frame, float x, float y, double angle, Flip flip);
/** As sprite_draw, with the frame scaled by scale (2.0 doubles it; the camera's zoom is the usual argument). Scaling is about the top-left corner: the drawn rectangle is x, y, frame_w*scale, frame_h*scale.
 *  @see sprite_draw, Camera
 */
void sprite_draw_scaled(const Sprite *s, int frame, float x, float y, float scale, double angle, Flip flip);

/* Draw with the texture's colors multiplied by tint (white sheet x tint = tint; alpha
   fades). Used for palette-free recoloring: layered white silhouettes tinted per entity.
   The texture's modulation is restored to neutral afterward, so other draws are unaffected. */
/** Draw one frame with every pixel's color multiplied by tint and its alpha by tint.a. A white sheet drawn with tint red comes out red, which is how one set of white silhouettes becomes many differently colored characters; a tint with alpha 128 draws the frame half transparent (fade-outs, ghosts); tinting red for a few frames is the classic damage flash. The texture's modulation is restored to neutral afterwards, so other draws of the same sheet are unaffected. No rotation or scaling.
 *  @see sprite_draw, rgba, Color
 */
void sprite_draw_tinted(const Sprite *s, int frame, float x, float y, Color tint, Flip flip);
