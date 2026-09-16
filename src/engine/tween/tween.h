#pragma once
#include <stdbool.h>

typedef struct Timer { float left; bool running; } Timer;
static inline void timer_start(Timer *t, float seconds) { t->left = seconds; t->running = true; }
static inline bool timer_tick(Timer *t, float dt) {   /* true on the tick it expires */
  if (!t->running) return false;
  t->left -= dt;
  if (t->left <= 0) { t->running = false; return true; }
  return false;
}

typedef float (*EaseFn)(float);
float ease_linear(float t);
float ease_in_quad(float t);
float ease_out_quad(float t);
float ease_in_out_quad(float t);
float ease_out_cubic(float t);
float ease_out_back(float t);
float ease_out_bounce(float t);

typedef struct Tween {
  float *target;
  float  from, to, duration, elapsed;
  EaseFn ease;
  bool   active;
} Tween;
void tween_start(Tween *tw, float *target, float to, float duration, EaseFn ease);
void tween_update(Tween *tw, float dt);
static inline bool tween_done(const Tween *tw) { return !tw->active; }

typedef struct Shake { float amount, decay; float ox, oy; unsigned seed; } Shake;
void shake_start(Shake *s, float amount, float decay_per_sec);
void shake_update(Shake *s, float dt);
