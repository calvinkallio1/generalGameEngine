#pragma once
#include <math.h>

typedef struct { float x, y; } Vec2;

static inline Vec2  v2(float x, float y)              { return (Vec2){ x, y }; }
static inline Vec2  v2_add(Vec2 a, Vec2 b)            { return (Vec2){ a.x + b.x, a.y + b.y }; }
static inline Vec2  v2_sub(Vec2 a, Vec2 b)            { return (Vec2){ a.x - b.x, a.y - b.y }; }
static inline Vec2  v2_scale(Vec2 a, float s)         { return (Vec2){ a.x * s, a.y * s }; }
static inline float v2_dot(Vec2 a, Vec2 b)            { return a.x * b.x + a.y * b.y; }
static inline float v2_len(Vec2 a)                    { return sqrtf(a.x * a.x + a.y * a.y); }
static inline Vec2  v2_norm(Vec2 a)                   { float l = v2_len(a); return l > 0 ? v2_scale(a, 1.0f / l) : a; }
static inline Vec2  v2_lerp(Vec2 a, Vec2 b, float t)  { return (Vec2){ a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t }; }
static inline float v2_dist(Vec2 a, Vec2 b)           { return v2_len(v2_sub(a, b)); }

static inline float lerpf(float a, float b, float t)      { return a + (b - a) * t; }
static inline float clampf(float v, float lo, float hi)   { return v < lo ? lo : (v > hi ? hi : v); }
static inline int   clampi(int v, int lo, int hi)         { return v < lo ? lo : (v > hi ? hi : v); }
static inline float signf(float v)                        { return v > 0 ? 1.0f : (v < 0 ? -1.0f : 0.0f); }
static inline float minf(float a, float b)                { return a < b ? a : b; }
static inline float maxf(float a, float b)                { return a > b ? a : b; }
/* Framerate-independent smoothing: fraction to move toward a target this tick. */
static inline float smooth_t(float smoothing, float dt)   { return smoothing <= 0 ? 1.0f : 1.0f - expf(-dt / smoothing); }
