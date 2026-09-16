#pragma once
#include "input.h"

#define ACTION_MAX 32
#define BINDS_PER_ACTION 3

typedef struct ActionMap {
  SDL_Scancode      keys[ACTION_MAX][BINDS_PER_ACTION];   /* SDL_SCANCODE_UNKNOWN = unbound */
  SDL_GamepadButton pad[ACTION_MAX];                      /* SDL_GAMEPAD_BUTTON_INVALID = unbound */
  SDL_GamepadAxis   axis[ACTION_MAX];                     /* SDL_GAMEPAD_AXIS_INVALID = none */
  float             axis_sign[ACTION_MAX];                /* +1 / -1: which direction counts */
} ActionMap;

static inline void action_map_init(ActionMap *m) {
  for (int a = 0; a < ACTION_MAX; ++a) {
    for (int i = 0; i < BINDS_PER_ACTION; ++i) m->keys[a][i] = SDL_SCANCODE_UNKNOWN;
    m->pad[a] = SDL_GAMEPAD_BUTTON_INVALID; m->axis[a] = SDL_GAMEPAD_AXIS_INVALID; m->axis_sign[a] = 0;
  }
}
static inline void action_bind_key(ActionMap *m, int a, SDL_Scancode key) {
  for (int i = 0; i < BINDS_PER_ACTION; ++i) if (m->keys[a][i] == SDL_SCANCODE_UNKNOWN) { m->keys[a][i] = key; return; }
}
static inline void action_bind_pad(ActionMap *m, int a, SDL_GamepadButton b)              { m->pad[a] = b; }
static inline void action_bind_axis(ActionMap *m, int a, SDL_GamepadAxis ax, float sign)  { m->axis[a] = ax; m->axis_sign[a] = sign; }

static inline bool action_down(const ActionMap *m, const Input *in, int a) {
  for (int i = 0; i < BINDS_PER_ACTION; ++i) if (m->keys[a][i] != SDL_SCANCODE_UNKNOWN && key_down(in, m->keys[a][i])) return true;
  if (m->pad[a] != SDL_GAMEPAD_BUTTON_INVALID && pad_down(in, m->pad[a])) return true;
  if (m->axis[a] != SDL_GAMEPAD_AXIS_INVALID && in->pad_axis[m->axis[a]] * m->axis_sign[a] > 0.5f) return true;
  return false;
}
static inline bool action_pressed(const ActionMap *m, const Input *in, int a) {
  for (int i = 0; i < BINDS_PER_ACTION; ++i) if (m->keys[a][i] != SDL_SCANCODE_UNKNOWN && key_pressed(in, m->keys[a][i])) return true;
  if (m->pad[a] != SDL_GAMEPAD_BUTTON_INVALID && pad_pressed(in, m->pad[a])) return true;
  return false;
}
static inline float action_axis(const ActionMap *m, const Input *in, int neg, int pos) {
  float v = (float)action_down(m, in, pos) - (float)action_down(m, in, neg);
  if (v == 0 && m->axis[pos] != SDL_GAMEPAD_AXIS_INVALID) {
    float a = in->pad_axis[m->axis[pos]] * m->axis_sign[pos];
    if (a > 0.2f || a < -0.2f) v = a;
  }
  return v;
}
