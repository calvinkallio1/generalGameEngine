#pragma once
#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include "input.h"
#include "scene.h"

#define SCENE_STACK_MAX 8
#define PENDING_MAX     8

/* Everything the engine needs to know about your game, passed once to engine_init. */
typedef struct EngineConfig {
  const char *title;
  int   logical_w, logical_h;  /* the resolution your game code thinks in (e.g. 640x360) */
  int   window_scale;          /* initial window = logical * scale (0 -> 2) */
  int   tick_rate;             /* fixed updates per second (0 -> 60) */
  void *user;                  /* your game's state; the engine stores it, never reads it */
} EngineConfig;

/* One requested change to the scene stack, applied later at a safe point. */
typedef enum { OP_PUSH, OP_POP, OP_REPLACE } SceneOp;
typedef struct PendingOp {
  SceneOp op;
  Scene  *scene;   /* NULL for OP_POP */
} PendingOp;

typedef struct Engine {
  SDL_Window   *window;
  SDL_Renderer *renderer;
  Input         input;
  bool          running;

  int      logical_w, logical_h;
  uint64_t ns_per_tick;   /* derived from tick_rate */
  float    dt;            /* seconds per tick, passed to Scene.update */
  void    *user;          /* game state; scenes cast this to their own type */

  Scene    *stack[SCENE_STACK_MAX];   /* stack[0] is bottom, stack[stack_len-1] is top */
  int       stack_len;
  PendingOp pending[PENDING_MAX];     /* queued stack changes, applied in order */
  int       pending_len;
} Engine;

bool engine_init(Engine *e, const EngineConfig *cfg);
void engine_run(Engine *e);
void engine_shutdown(Engine *e);

/* Scenes call these. They only queue; nothing changes until the engine applies them. */
void engine_push(Engine *e, Scene *s);
void engine_pop(Engine *e);
void engine_replace(Engine *e, Scene *s);
static inline void engine_quit(Engine *e) { e->running = false; }
