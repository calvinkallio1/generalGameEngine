#include "tween.h"
#include <math.h>

float ease_linear(float t)      { return t; }
float ease_in_quad(float t)     { return t * t; }
float ease_out_quad(float t)    { return 1 - (1 - t) * (1 - t); }
float ease_in_out_quad(float t) { return t < 0.5f ? 2 * t * t : 1 - powf(-2 * t + 2, 2) / 2; }
float ease_out_cubic(float t)   { return 1 - powf(1 - t, 3); }
float ease_out_back(float t)    { const float c1 = 1.70158f, c3 = c1 + 1; return 1 + c3 * powf(t - 1, 3) + c1 * powf(t - 1, 2); }
float ease_out_bounce(float t) {
  const float n1 = 7.5625f, d1 = 2.75f;
  if (t < 1 / d1)         return n1 * t * t;
  else if (t < 2 / d1)    { t -= 1.5f / d1;   return n1 * t * t + 0.75f; }
  else if (t < 2.5f / d1) { t -= 2.25f / d1;  return n1 * t * t + 0.9375f; }
  else                    { t -= 2.625f / d1; return n1 * t * t + 0.984375f; }
}

void tween_start(Tween *tw, float *target, float to, float duration, EaseFn ease) {
  tw->target = target; tw->from = *target; tw->to = to;
  tw->duration = duration > 0 ? duration : 0.0001f; tw->elapsed = 0;
  tw->ease = ease ? ease : ease_linear; tw->active = true;
}

void tween_update(Tween *tw, float dt) {
  if (!tw->active) return;
  tw->elapsed += dt;
  float t = tw->elapsed / tw->duration;
  if (t >= 1) { t = 1; tw->active = false; }
  *tw->target = tw->from + (tw->to - tw->from) * tw->ease(t);
}

static float frand(unsigned *seed) {
  *seed = *seed * 1664525u + 1013904223u;
  return ((*seed >> 8) & 0xFFFF) / 32767.5f - 1.0f;
}

void shake_start(Shake *s, float amount, float decay_per_sec) {
  s->amount = amount; s->decay = decay_per_sec;
  if (!s->seed) s->seed = 12345;
}

void shake_update(Shake *s, float dt) {
  if (s->amount <= 0) { s->ox = s->oy = 0; return; }
  s->ox = frand(&s->seed) * s->amount;
  s->oy = frand(&s->seed) * s->amount;
  s->amount -= s->decay * dt;
  if (s->amount < 0) s->amount = 0;
}
