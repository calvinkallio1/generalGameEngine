#pragma once
#include <stdbool.h>

/** A countdown in seconds. Pointer-free, so it may live in GameState (a spawn timer, an invulnerability window) or in a scene static (a "press start" blink). Start with timer_start(), advance with timer_tick() once per update; the tick that reaches zero returns true exactly once.
 *  @field left seconds remaining
 *  @field running true between timer_start() and expiry
 *  @see timer_start, timer_tick
 */
typedef struct Timer { float left; bool running; } Timer;
/** Start (or restart) a countdown of seconds. Restarting a running timer simply resets it.
 *  @see timer_tick, Timer
 */
static inline void timer_start(Timer *t, float seconds) { t->left = seconds; t->running = true; }
/** Advance a timer by dt. Returns true on the single tick it reaches zero, then stops; returns false while counting and while stopped. The usual shape: if (timer_tick(&g->spawn, dt)) { spawn(); timer_start(&g->spawn, 2.0f); }. For "is the effect still active" read t->running instead.
 *  @return true once, on the tick the timer expires.
 *  @see timer_start, Timer
 */
static inline bool timer_tick(Timer *t, float dt) {   /* true on the tick it expires */
  if (!t->running) return false;
  t->left -= dt;
  if (t->left <= 0) { t->running = false; return true; }
  return false;
}

/** An easing curve: maps progress t in 0..1 to an eased 0..1. Pass one of the ease_* functions to tween_start(); write your own with the same signature for custom motion.
 *  @see ease_linear, ease_out_quad, tween_start
 */
typedef float (*EaseFn)(float);
/** Constant speed: returns t unchanged. Mechanical motion, progress bars, fades where you want no acceleration.
 *  @return t.
 *  @see tween_start, EaseFn
 */
float ease_linear(float t);
/** Starts slow, speeds up (t squared). Things that fall away or launch: a menu sliding off screen, a charge building.
 *  @return the eased value.
 *  @see ease_out_quad, ease_in_out_quad
 */
float ease_in_quad(float t);
/** Starts fast, slows to a stop. The default choice for things arriving: a panel sliding in, a camera settling, a counter rolling up.
 *  @return the eased value.
 *  @see ease_out_cubic, ease_in_quad
 */
float ease_out_quad(float t);
/** Slow at both ends, fast in the middle. Camera pans, cutscene walks, anything that should look deliberate.
 *  @return the eased value.
 *  @see ease_out_quad
 */
float ease_in_out_quad(float t);
/** Like ease_out_quad but decelerates harder, so it arrives with more snap.
 *  @return the eased value.
 *  @see ease_out_quad
 */
float ease_out_cubic(float t);
/** Overshoots the target slightly and settles back. Pop-ups, score bumps, anything that should feel springy. Values briefly exceed 1.
 *  @return the eased value, up to about 1.1.
 *  @see ease_out_bounce
 */
float ease_out_back(float t);
/** Arrives and bounces a few times before resting, like a dropped ball. Coins landing, letters dropping into a title.
 *  @return the eased value.
 *  @see ease_out_back
 */
float ease_out_bounce(float t);

/** Animates one float from its current value to a target over a duration with an easing curve, by writing through a pointer. Presentation state: keep tweens in scene statics and point them at scene statics (a panel's x, a fade alpha, a zoom) or at GameState floats that are purely visual. Start with tween_start(), advance with tween_update() once per update; tween_done() reports completion. One Tween animates one float; a Vec2 needs two.
 *  @field target the float being animated; must outlive the tween
 *  @field from value when the tween started
 *  @field to destination
 *  @field duration seconds
 *  @field elapsed seconds so far
 *  @field ease curve applied to elapsed/duration
 *  @field active true until elapsed reaches duration
 *  @see tween_start, tween_update, tween_done, EaseFn, gge-tween
 */
typedef struct Tween {
  float *target;
  float  from, to, duration, elapsed;
  EaseFn ease;
  bool   active;
} Tween;
/** Animate *target from its current value to `to` over duration seconds using ease (NULL = linear). target must stay valid for the tween's life (a scene static or GameState field, never a local). Starting a tween that is already running restarts it from the current value, which makes retargeting smooth. A duration of 0 completes on the next update.
 *  @see tween_update, tween_done, Tween
 */
void tween_start(Tween *tw, float *target, float to, float duration, EaseFn ease);
/** Advance a tween by dt, writing the new value through its target pointer. Call from update, once per tick, for every tween you own; an inactive tween costs nothing. On the tick it finishes the target is set exactly to `to`.
 *  @see tween_start, tween_done
 */
void tween_update(Tween *tw, float dt);
/** True once the tween has finished (or never started). Chain tweens by starting the next one when this returns true.
 *  @return !tw->active.
 *  @see tween_start, tween_update
 */
static inline bool tween_done(const Tween *tw) { return !tw->active; }

/** Screen shake: a random pixel offset whose amplitude decays over time. Keep one in a scene static; shake_start() on impacts, shake_update() every tick, then add ox, oy to the camera's offset (cam.offset = v2(shake.ox, shake.oy)) or to every draw position. Stacking hits just call shake_start again with a larger amount.
 *  @field amount current amplitude in pixels; 0 when idle
 *  @field decay amplitude lost per second
 *  @field ox this tick's x offset
 *  @field oy this tick's y offset
 *  @field seed private random state
 *  @see shake_start, shake_update, Camera
 */
typedef struct Shake { float amount, decay; float ox, oy; unsigned seed; } Shake;
/** Begin a screen shake of the given pixel amplitude, decaying by decay_per_sec each second (amount 6, decay 20 is a solid hit that lasts 0.3 s). Calling it while shaking sets the new amount, so a bigger hit overrides a smaller one.
 *  @see shake_update, Shake
 */
void shake_start(Shake *s, float amount, float decay_per_sec);
/** Advance the shake: pick this tick's random offset into s->ox / s->oy and decay the amplitude. Call from update, then read the offsets in render or copy them to the camera.
 *  @see shake_start, Camera
 */
void shake_update(Shake *s, float dt);
