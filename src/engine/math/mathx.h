#pragma once
#include <math.h>

/** A 2D vector of floats: positions, velocities, directions, sizes. The engine's currency for anything with an x and a y in world or screen space. Pointer-free and tiny, so it lives directly inside entities in GameState. All v2_* helpers take and return Vec2 by value. Same layout as Point.
 *  @field x horizontal component
 *  @field y vertical component (positive is down on screen)
 *  @see v2, v2_add, v2_scale, v2_norm, gge-math
 */
typedef struct { float x, y; } Vec2;

/** Build a Vec2 from two floats. Equivalent to (Vec2){ x, y } but readable inside an expression: v2_add(pos, v2(0, -8)).
 *  @return the vector.
 *  @see Vec2
 */
static inline Vec2  v2(float x, float y)              { return (Vec2){ x, y }; }
/** Component-wise sum a + b. Moving a position by a displacement: pos = v2_add(pos, v2_scale(vel, dt)).
 *  @return a + b.
 *  @see v2_sub, v2_scale
 */
static inline Vec2  v2_add(Vec2 a, Vec2 b)            { return (Vec2){ a.x + b.x, a.y + b.y }; }
/** Component-wise difference a - b: the vector from b to a. v2_sub(target, pos) points from pos toward target.
 *  @return a - b.
 *  @see v2_add, v2_dist, v2_norm
 */
static inline Vec2  v2_sub(Vec2 a, Vec2 b)            { return (Vec2){ a.x - b.x, a.y - b.y }; }
/** Multiply both components by s. Scale a direction to a speed, a velocity to a displacement (times dt), or negate with -1.
 *  @return a * s.
 *  @see v2_norm, v2_add
 */
static inline Vec2  v2_scale(Vec2 a, float s)         { return (Vec2){ a.x * s, a.y * s }; }
/** Dot product a.x*b.x + a.y*b.y. For unit vectors it is the cosine of the angle between them: positive means roughly the same direction (is the enemy in front of me), zero means perpendicular. Also the projection of a onto b's direction when b is unit length.
 *  @return the scalar product.
 *  @see v2_norm, v2_len
 */
static inline float v2_dot(Vec2 a, Vec2 b)            { return a.x * b.x + a.y * b.y; }
/** Length (magnitude) of a: sqrt(x*x + y*y). Speed of a velocity, distance of a displacement. Involves a square root; compare squared lengths (v2_dot(a, a)) when only ordering matters.
 *  @return the length.
 *  @see v2_dist, v2_norm, v2_dot
 */
static inline float v2_len(Vec2 a)                    { return sqrtf(a.x * a.x + a.y * a.y); }
/** a scaled to length 1, or a itself (the zero vector) when its length is 0, so there is never a division by zero. Turns any vector into a pure direction: normalize the input vector before multiplying by speed so diagonals are not faster, and normalize v2_sub(target, pos) to chase something.
 *  @return the unit vector, or zero.
 *  @see v2_scale, v2_len, action_axis
 */
static inline Vec2  v2_norm(Vec2 a)                   { float l = v2_len(a); return l > 0 ? v2_scale(a, 1.0f / l) : a; }
/** Linear interpolation: a when t = 0, b when t = 1, the straight line between for values in between; t outside 0..1 extrapolates. Smooth rendering (v2_lerp(prev, now, alpha)), easing along a path, camera smoothing.
 *  @return a + (b - a) * t.
 *  @see lerpf, smooth_t
 */
static inline Vec2  v2_lerp(Vec2 a, Vec2 b, float t)  { return (Vec2){ a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t }; }
/** Distance between two points: v2_len(v2_sub(a, b)). Range checks, pickup radius, "is the player near the door". For a circle-vs-circle test use circle_overlaps(), which avoids the square root.
 *  @return the distance.
 *  @see v2_len, circle_overlaps
 */
static inline float v2_dist(Vec2 a, Vec2 b)           { return v2_len(v2_sub(a, b)); }

/** Linear interpolation of floats: a when t = 0, b when t = 1. Health bar animation, volume ramps, anything that slides between two numbers.
 *  @return a + (b - a) * t.
 *  @see v2_lerp, smooth_t, clampf
 */
static inline float lerpf(float a, float b, float t)      { return a + (b - a) * t; }
/** Limit v to the range lo..hi. Keeping a position inside the world, health inside 0..max, a menu index inside its list.
 *  @return lo if v < lo, hi if v > hi, else v.
 *  @see clampi, minf, maxf
 */
static inline float clampf(float v, float lo, float hi)   { return v < lo ? lo : (v > hi ? hi : v); }
/** Integer clamp of v to lo..hi.
 *  @return lo if v < lo, hi if v > hi, else v.
 *  @see clampf
 */
static inline int   clampi(int v, int lo, int hi)         { return v < lo ? lo : (v > hi ? hi : v); }
/** Sign of v as a float: 1 for positive, -1 for negative, 0 for zero. Facing direction from a velocity, or which way to apply friction.
 *  @return -1, 0 or 1.
 *  @see clampf
 */
static inline float signf(float v)                        { return v > 0 ? 1.0f : (v < 0 ? -1.0f : 0.0f); }
/** The smaller of a and b.
 *  @return min(a, b).
 *  @see maxf, clampf
 */
static inline float minf(float a, float b)                { return a < b ? a : b; }
/** The larger of a and b.
 *  @return max(a, b).
 *  @see minf, clampf
 */
static inline float maxf(float a, float b)                { return a > b ? a : b; }
/* Framerate-independent smoothing: fraction to move toward a target this tick. */
/** Framerate-independent smoothing: the fraction of the remaining distance to move toward a target this tick, given a time constant. value = lerpf(value, target, smooth_t(0.2f, dt)) approaches target with the same feel at any tick rate, reaching about 63% of the way in `smoothing` seconds. 0 or negative smoothing snaps (returns 1). camera_follow() is built on it.
 *  @return a fraction in 0..1.
 *  @see lerpf, v2_lerp, camera_follow
 */
static inline float smooth_t(float smoothing, float dt)   { return smoothing <= 0 ? 1.0f : 1.0f - expf(-dt / smoothing); }
