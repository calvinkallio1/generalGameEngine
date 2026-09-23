#pragma once
#include "input.h"

/* Maps game-defined action ids (an enum in your game) to keys, pad buttons and axes,
 * so game code says action_pressed(ACT_JUMP) instead of naming a key. */
/** Number of distinct actions an ActionMap can hold; your action enum's values must stay below it.
 *  @see ActionMap, action_bind_key
 */
#define ACTION_MAX 32
/** Keyboard keys per action in an ActionMap; further action_bind_key calls for the same action are ignored.
 *  @see action_bind_key, ActionMap
 */
#define BINDS_PER_ACTION 3

/** A table from your game's action ids (an enum you define, values 0..ACTION_MAX-1) to the physical inputs that trigger them: up to three keys, one gamepad button and one signed gamepad axis per action. Scenes then ask action_down(&map, in, ACT_JUMP) and never name a key, which is what makes rebinding, alternate layouts and controller support free. Keep one ActionMap per control scheme, usually a static in the gameplay scene filled in on_enter, or one in GameState if bindings should be saved (the struct is pointer-free).
 *  @field keys per action, up to BINDS_PER_ACTION keys; KEY_NONE marks an empty slot
 *  @field pad per action, one gamepad button or PAD_NONE
 *  @field axis per action, one gamepad axis or AXIS_NONE
 *  @field axis_sign +1 or -1: which direction of the axis counts as this action (a left stick pushed left is AXIS_LEFT_X with sign -1)
 *  @see action_map_init, action_bind_key, action_down, action_axis, gge-input
 */
typedef struct ActionMap {
  Key       keys[ACTION_MAX][BINDS_PER_ACTION];   /* KEY_NONE = unbound */
  PadButton pad[ACTION_MAX];                      /* PAD_NONE = unbound */
  PadAxis   axis[ACTION_MAX];                     /* AXIS_NONE = none */
  float     axis_sign[ACTION_MAX];                /* +1 / -1: which direction counts */
} ActionMap;

/** Clear an ActionMap so every action is unbound. Call once (in on_enter, or at new game) before binding; a zeroed struct is not the same as an initialized one because KEY_NONE, PAD_NONE and AXIS_NONE are not zero.
 *  @see action_bind_key, ActionMap
 */
static inline void action_map_init(ActionMap *m) {
  for (int a = 0; a < ACTION_MAX; ++a) {
    for (int i = 0; i < BINDS_PER_ACTION; ++i) m->keys[a][i] = KEY_NONE;
    m->pad[a] = PAD_NONE; m->axis[a] = AXIS_NONE; m->axis_sign[a] = 0;
  }
}
/** Bind action a (your own enum value, < ACTION_MAX) to a keyboard key. Each action holds up to three keys, filled in order; a fourth call is ignored. Bind both arrows and WASD to the same action so either works: action_bind_key(&m, ACT_LEFT, KEY_LEFT); action_bind_key(&m, ACT_LEFT, KEY_A). To rebind, call action_map_init and bind again.
 *  @see action_bind_pad, action_bind_axis, action_down, action_pressed
 */
static inline void action_bind_key(ActionMap *m, int a, Key key) {
  for (int i = 0; i < BINDS_PER_ACTION; ++i) if (m->keys[a][i] == KEY_NONE) { m->keys[a][i] = key; return; }
}
/** Bind action a to a gamepad button, replacing any previous button for that action. Use positional names (PAD_SOUTH for confirm/jump) so the binding is right on every controller.
 *  @see action_bind_key, action_bind_axis, PadButton
 */
static inline void action_bind_pad(ActionMap *m, int a, PadButton b)              { m->pad[a] = b; }
/** Bind action a to a gamepad axis with a sign, so a stick direction counts as the action past a dead zone. sign +1 for positive travel (right, down, trigger pulled), -1 for negative (left, up). action_down treats the axis as held beyond 0.5; action_axis returns its analog value.
 *  @see action_axis, action_down, PadAxis
 */
static inline void action_bind_axis(ActionMap *m, int a, PadAxis ax, float sign)  { m->axis[a] = ax; m->axis_sign[a] = sign; }

/** True while any input bound to the action is held: any of its keys, its pad button, or its axis pushed past 0.5 in the bound direction. The action-level equivalent of key_down().
 *  @return true while held.
 *  @see action_pressed, action_axis, key_down
 */
static inline bool action_down(const ActionMap *m, const Input *in, int a) {
  for (int i = 0; i < BINDS_PER_ACTION; ++i) if (m->keys[a][i] != KEY_NONE && key_down(in, m->keys[a][i])) return true;
  if (m->pad[a] != PAD_NONE && pad_down(in, m->pad[a])) return true;
  if (m->axis[a] != AXIS_NONE && in->pad_axis[m->axis[a]] * m->axis_sign[a] > 0.5f) return true;
  return false;
}
/** True only on the frame any key or pad button bound to the action went down. Axes are not considered (a stick has no "press" edge). The action-level equivalent of key_pressed().
 *  @return true on the press frame.
 *  @see action_down, key_pressed
 */
static inline bool action_pressed(const ActionMap *m, const Input *in, int a) {
  for (int i = 0; i < BINDS_PER_ACTION; ++i) if (m->keys[a][i] != KEY_NONE && key_pressed(in, m->keys[a][i])) return true;
  if (m->pad[a] != PAD_NONE && pad_pressed(in, m->pad[a])) return true;
  return false;
}
/** -1..1 from a pair of actions: -1 while neg is down, +1 while pos is down, 0 when both or neither, or the analog stick value (past a 0.2 dead zone) if pos has an axis bound and no digital input is active. Call once per axis: x = action_axis(&m, in, ACT_LEFT, ACT_RIGHT); y = action_axis(&m, in, ACT_UP, ACT_DOWN). Normalize the resulting vector before scaling by speed so diagonals are not faster: v2_scale(v2_norm(v2(x, y)), speed).
 *  @return a value in -1..1.
 *  @see action_down, action_bind_axis, v2_norm
 */
static inline float action_axis(const ActionMap *m, const Input *in, int neg, int pos) {
  float v = (float)action_down(m, in, pos) - (float)action_down(m, in, neg);
  if (v == 0 && m->axis[pos] != AXIS_NONE) {
    float a = in->pad_axis[m->axis[pos]] * m->axis_sign[pos];
    if (a > 0.2f || a < -0.2f) v = a;
  }
  return v;
}
