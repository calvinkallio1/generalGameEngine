#pragma once
#include "input.h"

/* Maps game-defined action ids (an enum in your game) to keys, pad buttons and axes,
 * so game code says action_pressed(ACT_JUMP) instead of naming a key. */
#define ACTION_MAX 32
#define BINDS_PER_ACTION 3

typedef struct ActionMap {
  Key       keys[ACTION_MAX][BINDS_PER_ACTION];   /* KEY_NONE = unbound */
  PadButton pad[ACTION_MAX];                      /* PAD_NONE = unbound */
  PadAxis   axis[ACTION_MAX];                     /* AXIS_NONE = none */
  float     axis_sign[ACTION_MAX];                /* +1 / -1: which direction counts */
} ActionMap;

static inline void action_map_init(ActionMap *m) {
  for (int a = 0; a < ACTION_MAX; ++a) {
    for (int i = 0; i < BINDS_PER_ACTION; ++i) m->keys[a][i] = KEY_NONE;
    m->pad[a] = PAD_NONE; m->axis[a] = AXIS_NONE; m->axis_sign[a] = 0;
  }
}
static inline void action_bind_key(ActionMap *m, int a, Key key) {
  for (int i = 0; i < BINDS_PER_ACTION; ++i) if (m->keys[a][i] == KEY_NONE) { m->keys[a][i] = key; return; }
}
static inline void action_bind_pad(ActionMap *m, int a, PadButton b)              { m->pad[a] = b; }
static inline void action_bind_axis(ActionMap *m, int a, PadAxis ax, float sign)  { m->axis[a] = ax; m->axis_sign[a] = sign; }

static inline bool action_down(const ActionMap *m, const Input *in, int a) {
  for (int i = 0; i < BINDS_PER_ACTION; ++i) if (m->keys[a][i] != KEY_NONE && key_down(in, m->keys[a][i])) return true;
  if (m->pad[a] != PAD_NONE && pad_down(in, m->pad[a])) return true;
  if (m->axis[a] != AXIS_NONE && in->pad_axis[m->axis[a]] * m->axis_sign[a] > 0.5f) return true;
  return false;
}
static inline bool action_pressed(const ActionMap *m, const Input *in, int a) {
  for (int i = 0; i < BINDS_PER_ACTION; ++i) if (m->keys[a][i] != KEY_NONE && key_pressed(in, m->keys[a][i])) return true;
  if (m->pad[a] != PAD_NONE && pad_pressed(in, m->pad[a])) return true;
  return false;
}
static inline float action_axis(const ActionMap *m, const Input *in, int neg, int pos) {
  float v = (float)action_down(m, in, pos) - (float)action_down(m, in, neg);
  if (v == 0 && m->axis[pos] != AXIS_NONE) {
    float a = in->pad_axis[m->axis[pos]] * m->axis_sign[pos];
    if (a > 0.2f || a < -0.2f) v = a;
  }
  return v;
}
