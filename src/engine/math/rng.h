#pragma once
#include <stdint.h>
#include <stdbool.h>

/** Advance a 64-bit random state and return the next raw value (splitmix64). The state is just a uint64_t you own, normally GameState.rng, seeded once at new game; every other rng_* call takes its address. Because the state is plain data it saves with the game and replays identically from the same seed: the same seed always produces the same level, the same loot, the same dice. The other helpers are what games call; this is the primitive underneath.
 *  @return a uniformly distributed 64-bit value.
 *  @see rng_float, rng_range, rng_chance, gge-math
 */
static inline uint64_t rng_next(uint64_t *s) {
  uint64_t z = (*s += 0x9E3779B97F4A7C15ULL);
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
  return z ^ (z >> 31);
}
/** A random float in [0, 1). Scale it yourself, or use rng_between() and rng_chance().
 *  @return a value from 0 up to but not including 1.
 *  @see rng_between, rng_chance, rng_next
 */
static inline float rng_float(uint64_t *s)                       { return (rng_next(s) >> 40) * (1.0f / 16777216.0f); } /* [0,1) */
/** A random integer in [lo, hi], both ends inclusive: rng_range(&g->rng, 1, 6) is a die. hi must be >= lo.
 *  @return an integer from lo to hi.
 *  @see rng_float, rng_between
 */
static inline int   rng_range(uint64_t *s, int lo, int hi)       { return lo + (int)(rng_next(s) % (uint64_t)(hi - lo + 1)); } /* [lo,hi] */
/** A random float in [lo, hi): spawn positions, particle speeds, timer jitter.
 *  @return a value from lo up to but not including hi.
 *  @see rng_float, rng_range
 */
static inline float rng_between(uint64_t *s, float lo, float hi) { return lo + rng_float(s) * (hi - lo); }
/** True with probability p (0..1): rng_chance(&g->rng, 0.25f) is a one-in-four drop.
 *  @return true p of the time.
 *  @see rng_float
 */
static inline bool  rng_chance(uint64_t *s, float p)             { return rng_float(s) < p; }
