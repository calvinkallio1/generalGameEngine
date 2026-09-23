#pragma once
/* Engine names for keys, mouse buttons, gamepad buttons and axes. Each value equals
 * the SDL code underneath, so they index the same tables; games just never see "SDL_". */
#include <SDL3/SDL.h>

/** Keyboard keys by physical position (scancodes): KEY_W is the key where W sits on a US layout, wherever the user's layout puts the letter, which keeps WASD usable on AZERTY and Dvorak keyboards. Pass to key_down(), key_pressed(), key_released() and action_bind_key(). KEY_NONE means "no key" in an ActionMap; KEY_COUNT sizes the Input arrays. Letters, digits, F1-F12, arrows, SPACE, ENTER, ESCAPE, TAB, BACKSPACE, DELETE, both SHIFT/CTRL/ALT, common punctuation and HOME/END/PAGEUP/PAGEDOWN are named. F3 and F11 are consumed by the engine (debug overlay, fullscreen) before scenes see them, though they still appear in Input.keys.
 *  @see key_down, key_pressed, action_bind_key, Input, gge-input
 */
typedef enum Key {
  KEY_A = SDL_SCANCODE_A, KEY_B = SDL_SCANCODE_B, KEY_C = SDL_SCANCODE_C, KEY_D = SDL_SCANCODE_D,
  KEY_E = SDL_SCANCODE_E, KEY_F = SDL_SCANCODE_F, KEY_G = SDL_SCANCODE_G, KEY_H = SDL_SCANCODE_H,
  KEY_I = SDL_SCANCODE_I, KEY_J = SDL_SCANCODE_J, KEY_K = SDL_SCANCODE_K, KEY_L = SDL_SCANCODE_L,
  KEY_M = SDL_SCANCODE_M, KEY_N = SDL_SCANCODE_N, KEY_O = SDL_SCANCODE_O, KEY_P = SDL_SCANCODE_P,
  KEY_Q = SDL_SCANCODE_Q, KEY_R = SDL_SCANCODE_R, KEY_S = SDL_SCANCODE_S, KEY_T = SDL_SCANCODE_T,
  KEY_U = SDL_SCANCODE_U, KEY_V = SDL_SCANCODE_V, KEY_W = SDL_SCANCODE_W, KEY_X = SDL_SCANCODE_X,
  KEY_Y = SDL_SCANCODE_Y, KEY_Z = SDL_SCANCODE_Z,
  KEY_0 = SDL_SCANCODE_0, KEY_1 = SDL_SCANCODE_1, KEY_2 = SDL_SCANCODE_2, KEY_3 = SDL_SCANCODE_3,
  KEY_4 = SDL_SCANCODE_4, KEY_5 = SDL_SCANCODE_5, KEY_6 = SDL_SCANCODE_6, KEY_7 = SDL_SCANCODE_7,
  KEY_8 = SDL_SCANCODE_8, KEY_9 = SDL_SCANCODE_9,
  KEY_F1 = SDL_SCANCODE_F1, KEY_F2 = SDL_SCANCODE_F2, KEY_F3 = SDL_SCANCODE_F3, KEY_F4 = SDL_SCANCODE_F4,
  KEY_F5 = SDL_SCANCODE_F5, KEY_F6 = SDL_SCANCODE_F6, KEY_F7 = SDL_SCANCODE_F7, KEY_F8 = SDL_SCANCODE_F8,
  KEY_F9 = SDL_SCANCODE_F9, KEY_F10 = SDL_SCANCODE_F10, KEY_F11 = SDL_SCANCODE_F11, KEY_F12 = SDL_SCANCODE_F12,
  KEY_UP = SDL_SCANCODE_UP, KEY_DOWN = SDL_SCANCODE_DOWN, KEY_LEFT = SDL_SCANCODE_LEFT, KEY_RIGHT = SDL_SCANCODE_RIGHT,
  KEY_SPACE = SDL_SCANCODE_SPACE, KEY_ENTER = SDL_SCANCODE_RETURN, KEY_ESCAPE = SDL_SCANCODE_ESCAPE,
  KEY_TAB = SDL_SCANCODE_TAB, KEY_BACKSPACE = SDL_SCANCODE_BACKSPACE, KEY_DELETE = SDL_SCANCODE_DELETE,
  KEY_LSHIFT = SDL_SCANCODE_LSHIFT, KEY_RSHIFT = SDL_SCANCODE_RSHIFT,
  KEY_LCTRL = SDL_SCANCODE_LCTRL, KEY_RCTRL = SDL_SCANCODE_RCTRL,
  KEY_LALT = SDL_SCANCODE_LALT, KEY_RALT = SDL_SCANCODE_RALT,
  KEY_MINUS = SDL_SCANCODE_MINUS, KEY_EQUALS = SDL_SCANCODE_EQUALS,
  KEY_LBRACKET = SDL_SCANCODE_LEFTBRACKET, KEY_RBRACKET = SDL_SCANCODE_RIGHTBRACKET,
  KEY_COMMA = SDL_SCANCODE_COMMA, KEY_PERIOD = SDL_SCANCODE_PERIOD, KEY_SLASH = SDL_SCANCODE_SLASH,
  KEY_GRAVE = SDL_SCANCODE_GRAVE, KEY_HOME = SDL_SCANCODE_HOME, KEY_END = SDL_SCANCODE_END,
  KEY_PAGEUP = SDL_SCANCODE_PAGEUP, KEY_PAGEDOWN = SDL_SCANCODE_PAGEDOWN,
  KEY_NONE = SDL_SCANCODE_UNKNOWN,
  KEY_COUNT = SDL_SCANCODE_COUNT,
} Key;

/** Mouse buttons for mouse_down(), mouse_pressed() and mouse_released().
 *  @field MOUSE_LEFT primary button
 *  @field MOUSE_MIDDLE wheel click
 *  @field MOUSE_RIGHT secondary button
 *  @see mouse_pressed, Input
 */
typedef enum MouseButton {
  MOUSE_LEFT   = SDL_BUTTON_LEFT,
  MOUSE_MIDDLE = SDL_BUTTON_MIDDLE,
  MOUSE_RIGHT  = SDL_BUTTON_RIGHT,
} MouseButton;

/** Gamepad buttons, named by position on the controller so one binding works on Xbox, PlayStation and Nintendo pads. Pass to pad_down(), pad_pressed() and action_bind_pad(). PAD_NONE means unbound in an ActionMap; PAD_BUTTON_COUNT sizes the Input arrays.
 *  @field PAD_SOUTH bottom face button: A on Xbox, Cross on PlayStation, B on Nintendo. The conventional confirm/jump.
 *  @field PAD_EAST right face button: B / Circle / A. The conventional cancel/back.
 *  @field PAD_WEST left face button: X / Square / Y
 *  @field PAD_NORTH top face button: Y / Triangle / X
 *  @field PAD_BACK Back / Share / Minus
 *  @field PAD_GUIDE the Xbox / PS / Home button; often intercepted by the OS
 *  @field PAD_START Start / Options / Plus. The conventional pause.
 *  @field PAD_LSTICK clicking the left stick
 *  @field PAD_RSTICK clicking the right stick
 *  @field PAD_LB left shoulder bumper
 *  @field PAD_RB right shoulder bumper
 *  @field PAD_DPAD_UP directional pad up
 *  @field PAD_DPAD_DOWN directional pad down
 *  @field PAD_DPAD_LEFT directional pad left
 *  @field PAD_DPAD_RIGHT directional pad right
 *  @field PAD_NONE no button (unbound)
 *  @field PAD_BUTTON_COUNT array size
 *  @see pad_down, pad_pressed, action_bind_pad, PadAxis
 */
typedef enum PadButton {
  PAD_SOUTH = SDL_GAMEPAD_BUTTON_SOUTH,   /* A on Xbox, Cross on PlayStation, B on Nintendo */
  PAD_EAST  = SDL_GAMEPAD_BUTTON_EAST,    /* B / Circle / A */
  PAD_WEST  = SDL_GAMEPAD_BUTTON_WEST,    /* X / Square / Y */
  PAD_NORTH = SDL_GAMEPAD_BUTTON_NORTH,   /* Y / Triangle / X */
  PAD_BACK  = SDL_GAMEPAD_BUTTON_BACK,
  PAD_GUIDE = SDL_GAMEPAD_BUTTON_GUIDE,
  PAD_START = SDL_GAMEPAD_BUTTON_START,
  PAD_LSTICK = SDL_GAMEPAD_BUTTON_LEFT_STICK, PAD_RSTICK = SDL_GAMEPAD_BUTTON_RIGHT_STICK,
  PAD_LB = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, PAD_RB = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
  PAD_DPAD_UP = SDL_GAMEPAD_BUTTON_DPAD_UP, PAD_DPAD_DOWN = SDL_GAMEPAD_BUTTON_DPAD_DOWN,
  PAD_DPAD_LEFT = SDL_GAMEPAD_BUTTON_DPAD_LEFT, PAD_DPAD_RIGHT = SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
  PAD_NONE = SDL_GAMEPAD_BUTTON_INVALID,
  PAD_BUTTON_COUNT = SDL_GAMEPAD_BUTTON_COUNT,
} PadButton;

/** Gamepad analog axes, indexes into Input.pad_axis. Sticks read -1..1 on each axis (negative = left or up), triggers 0..1. A 0.2 dead zone is applied by the engine so a resting stick reads exactly 0. AXIS_NONE means none in an ActionMap; AXIS_COUNT sizes the array.
 *  @field AXIS_LEFT_X left stick horizontal
 *  @field AXIS_LEFT_Y left stick vertical (negative = up)
 *  @field AXIS_RIGHT_X right stick horizontal
 *  @field AXIS_RIGHT_Y right stick vertical
 *  @field AXIS_LT left trigger, 0..1
 *  @field AXIS_RT right trigger, 0..1
 *  @field AXIS_NONE no axis (unbound)
 *  @field AXIS_COUNT array size
 *  @see action_bind_axis, action_axis, Input
 */
typedef enum PadAxis {
  AXIS_LEFT_X  = SDL_GAMEPAD_AXIS_LEFTX,  AXIS_LEFT_Y  = SDL_GAMEPAD_AXIS_LEFTY,
  AXIS_RIGHT_X = SDL_GAMEPAD_AXIS_RIGHTX, AXIS_RIGHT_Y = SDL_GAMEPAD_AXIS_RIGHTY,
  AXIS_LT = SDL_GAMEPAD_AXIS_LEFT_TRIGGER, AXIS_RT = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,
  AXIS_NONE = SDL_GAMEPAD_AXIS_INVALID,
  AXIS_COUNT = SDL_GAMEPAD_AXIS_COUNT,
} PadAxis;
