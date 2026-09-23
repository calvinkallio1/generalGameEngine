#pragma once
#include <stdbool.h>
#include "keys.h"

/** Per-frame snapshot of every input device. The engine fills it once per frame at the top of the loop; scenes only read it, either as the const Input * passed to handle_input or as &e->input inside update. The prev_* copies are last frame's values, which is what makes "pressed this frame" a comparison rather than an event queue. Because the snapshot is taken per displayed frame, a key tapped and released between two frames can be missed at very low frame rates; at 60 fps this does not happen in practice.
 *  @field keys true for every key currently held, indexed by Key
 *  @field prev_keys keys as of the previous frame
 *  @field mouse_x mouse x in logical pixels (already converted from the window, so it lines up with what you draw even when the window is resized or letterboxed)
 *  @field mouse_y mouse y in logical pixels
 *  @field wheel scroll wheel movement this frame: positive away from the user (up), negative toward; 0 when idle. Accumulates if several wheel events arrive in one frame.
 *  @field mouse bitmask of held mouse buttons; query with mouse_down() rather than reading it
 *  @field prev_mouse the mask as of the previous frame
 *  @field pad_connected true while a gamepad is open (requires EngineConfig.gamepad)
 *  @field pad_buttons true for every held gamepad button, indexed by PadButton
 *  @field prev_pad_buttons buttons as of the previous frame
 *  @field pad_axis stick and trigger values, indexed by PadAxis, -1..1 with a 0.2 dead zone already applied (values inside the dead zone read as exactly 0). Sticks: negative is left/up. Triggers: 0..1.
 *  @field text characters typed this frame as a UTF-8 string, only while engine_text_input() is on; empty otherwise. At most 31 bytes.
 *  @field quit_requested true once the window's close button (or the OS quit shortcut) was used; the engine ends the loop itself, so scenes rarely need it
 *  @see key_down, key_pressed, mouse_pressed, pad_down, action_down, engine_text_input, gge-input
 */
typedef struct Input {
  bool  keys[KEY_COUNT];
  bool  prev_keys[KEY_COUNT];

  float mouse_x, mouse_y;          /* logical coordinates */
  float wheel;
  unsigned mouse, prev_mouse;      /* button bitmasks */

  bool  pad_connected;
  bool  pad_buttons[PAD_BUTTON_COUNT];
  bool  prev_pad_buttons[PAD_BUTTON_COUNT];
  float pad_axis[AXIS_COUNT];      /* -1..1, dead zone applied */

  char  text[32];                  /* typed characters this frame (only while engine_text_input is on) */
  bool  quit_requested;
} Input;

/** True while the key is held. Use for continuous actions: walking, holding a charge, scrolling. Because it is true every frame the key stays down, never use it for "open the menu" style reactions; that is key_pressed().
 *  @return true if in->keys[k] is set this frame.
 *  @see key_pressed, key_released, action_down, gge-input
 */
static inline bool key_down(const Input *in, Key k)     { return in->keys[k]; }
/** True only on the frame the key went down. Use for one-shot reactions (open a menu, jump, confirm, advance dialog). Holding the key does not repeat it; a second press requires a release first.
 *  @return true if the key is down now and was up last frame.
 *  @see key_down, key_released, action_pressed
 */
static inline bool key_pressed(const Input *in, Key k)  { return in->keys[k] && !in->prev_keys[k]; }
/** True only on the frame the key came up. Use for release-triggered mechanics: a variable-height jump that cuts short when the button is let go, a charged shot that fires on release.
 *  @return true if the key is up now and was down last frame.
 *  @see key_pressed, key_down
 */
static inline bool key_released(const Input *in, Key k) { return !in->keys[k] && in->prev_keys[k]; }

/** True while the mouse button is held. Position is in->mouse_x / in->mouse_y in logical pixels, so comparing it against a Rect you drew is exact regardless of window size; use rect_contains() for the test. For a world-space position under a camera, convert with camera_to_world().
 *  @return true if button b is held this frame.
 *  @see mouse_pressed, mouse_released, rect_contains, camera_to_world
 */
static inline bool mouse_down(const Input *in, MouseButton b)     { return (in->mouse & SDL_BUTTON_MASK(b)) != 0; }
/** True only on the frame the button went down. The usual "click" test: mouse_pressed(in, MOUSE_LEFT) && rect_contains(button_rect, v2(in->mouse_x, in->mouse_y)).
 *  @return true if button b is down now and was up last frame.
 *  @see mouse_down, mouse_released
 */
static inline bool mouse_pressed(const Input *in, MouseButton b)  { return (in->mouse & SDL_BUTTON_MASK(b)) && !(in->prev_mouse & SDL_BUTTON_MASK(b)); }
/** True only on the frame the button came up. Use to end a drag or a slingshot pull.
 *  @return true if button b is up now and was down last frame.
 *  @see mouse_pressed, mouse_down
 */
static inline bool mouse_released(const Input *in, MouseButton b) { return !(in->mouse & SDL_BUTTON_MASK(b)) && (in->prev_mouse & SDL_BUTTON_MASK(b)); }

/** True while the gamepad button is held (requires EngineConfig.gamepad and a connected pad; false otherwise). Axes are in in->pad_axis[AXIS_*] as -1..1. Button names are positional (PAD_SOUTH is A on Xbox, Cross on PlayStation, B on Nintendo) so the same code works on every controller.
 *  @return true if button b is held this frame.
 *  @see pad_pressed, PadButton, PadAxis
 */
static inline bool pad_down(const Input *in, PadButton b)    { return in->pad_buttons[b]; }
/** True only on the frame the gamepad button went down.
 *  @return true if button b is down now and was up last frame.
 *  @see pad_down
 */
static inline bool pad_pressed(const Input *in, PadButton b) { return in->pad_buttons[b] && !in->prev_pad_buttons[b]; }
