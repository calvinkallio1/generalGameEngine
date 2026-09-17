#pragma once
#include <stdbool.h>

typedef struct Timer { float left; bool running; } Timer;
/** Start a countdown of seconds.
 *  @see timer_tick
 */
static inline void timer_start(Timer *t, float seconds) { t->left = seconds; t->running = true; }
/** Advance a timer by dt. Returns true on the single tick it reaches zero, then stops.
 *  @see timer_start
 */
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
/** Animate *target from its current value to `to` over duration seconds using ease. target must stay valid for the tween's life (a scene static or GameState field).
 *  @see tween_update, tween_done
 */
void tween_start(Tween *tw, float *target, float to, float duration, EaseFn ease);
/** Advance a tween by dt, writing the new value through its target pointer. Call from update.
 *  @see tween_start
 */
void tween_update(Tween *tw, float dt);
/** True once the tween has finished (or never started).
 *  @see tween_start
 */
static inline bool tween_done(const Tween *tw) { return !tw->active; }

typedef struct Shake { float amount, decay; float ox, oy; unsigned seed; } Shake;
/** Begin a screen shake of the given pixel amplitude, decaying by decay_per_sec each second.
 *  @see shake_update
 */
void shake_start(Shake *s, float amount, float decay_per_sec);
/** Advance the shake; read s->offset and add it to the camera or draw positions.
 *  @see shake_start
 */
void shake_update(Shake *s, float dt);
