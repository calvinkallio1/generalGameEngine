#pragma once
#include <stdbool.h>
#include "keys.h"

/* Per-frame snapshot of every input device. The engine fills it once per frame;
 * scenes only read it. prev_* copies make edge detection a comparison. */
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

static inline bool key_down(const Input *in, Key k)     { return in->keys[k]; }
static inline bool key_pressed(const Input *in, Key k)  { return in->keys[k] && !in->prev_keys[k]; }
static inline bool key_released(const Input *in, Key k) { return !in->keys[k] && in->prev_keys[k]; }

static inline bool mouse_down(const Input *in, MouseButton b)     { return (in->mouse & SDL_BUTTON_MASK(b)) != 0; }
static inline bool mouse_pressed(const Input *in, MouseButton b)  { return (in->mouse & SDL_BUTTON_MASK(b)) && !(in->prev_mouse & SDL_BUTTON_MASK(b)); }
static inline bool mouse_released(const Input *in, MouseButton b) { return !(in->mouse & SDL_BUTTON_MASK(b)) && (in->prev_mouse & SDL_BUTTON_MASK(b)); }

static inline bool pad_down(const Input *in, PadButton b)    { return in->pad_buttons[b]; }
static inline bool pad_pressed(const Input *in, PadButton b) { return in->pad_buttons[b] && !in->prev_pad_buttons[b]; }
