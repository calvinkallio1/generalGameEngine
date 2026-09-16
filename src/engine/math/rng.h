#pragma once
#include <stdint.h>
#include <stdbool.h>

static inline uint64_t rng_next(uint64_t *s) {
  uint64_t z = (*s += 0x9E3779B97F4A7C15ULL);
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
  return z ^ (z >> 31);
}
static inline float rng_float(uint64_t *s)                       { return (rng_next(s) >> 40) * (1.0f / 16777216.0f); } /* [0,1) */
static inline int   rng_range(uint64_t *s, int lo, int hi)       { return lo + (int)(rng_next(s) % (uint64_t)(hi - lo + 1)); } /* [lo,hi] */
static inline float rng_between(uint64_t *s, float lo, float hi) { return lo + rng_float(s) * (hi - lo); }
static inline bool  rng_chance(uint64_t *s, float p)             { return rng_float(s) < p; }
