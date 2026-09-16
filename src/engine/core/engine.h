#pragma once
#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include "types.h"
#include "types.h"
#include "input.h"
#include "scene.h"
#include "save.h"

#define ENGINE_VERSION_MAJOR 0
#define ENGINE_VERSION_MINOR 1
#define ENGINE_VERSION_PATCH 0

#define SCENE_STACK_MAX 8
#define PENDING_MAX     8

/* Everything the engine needs to know about your game, passed once to engine_init.
 * Zero fields take defaults. */
typedef struct EngineConfig {
  const char *title;
  int   logical_w, logical_h;  /* the resolution your game code thinks in (0 -> 640x360) */
  int   window_scale;          /* initial window = logical * scale (0 -> 2) */
  int   tick_rate;             /* fixed updates per second (0 -> 60) */
  void *user;                  /* your game's state; the engine stores it, never reads it */

  const char *org, *app;       /* per-user save directory via SDL_GetPrefPath; NULL -> cwd */
  bool   audio;                /* open an audio device */
  bool   text;                 /* init SDL_ttf (only if built with ENGINE_WITH_TTF) */
  bool   gamepad;              /* open gamepads as they appear */
  Scene *pause_scene;          /* pushed automatically when the window loses focus (NULL = don't) */
  Color  clear_color;          /* zero alpha -> dark blue-grey */
} EngineConfig;

typedef enum { OP_PUSH, OP_POP, OP_REPLACE } SceneOp;
typedef struct PendingOp { SceneOp op; Scene *scene; } PendingOp;

typedef struct Engine {
  SDL_Window   *window;      /* internal */
  SDL_Renderer *renderer;    /* internal: games draw through draw.h */
  Input         input;
  bool          running;

  int      logical_w, logical_h;
  int      window_w, window_h;    /* real window size, tracked on resize */
  uint64_t ns_per_tick;
  float    dt;                    /* seconds per tick, passed to Scene.update */
  void    *user;                  /* game state; scenes cast this to their own type */
  Save     save;

  bool  has_audio, has_text;
  bool  focused;
  bool  debug_overlay;            /* F3 */
  float fps;                      /* smoothed */
  uint64_t frame;
  Color clear_color;
  Scene *pause_scene;

  Scene    *stack[SCENE_STACK_MAX];
  int       stack_len;
  PendingOp pending[PENDING_MAX];
  int       pending_len;
} Engine;

bool engine_init(Engine *e, const EngineConfig *cfg);
void engine_run(Engine *e);
void engine_shutdown(Engine *e);

void engine_push(Engine *e, Scene *s);
void engine_pop(Engine *e);
void engine_replace(Engine *e, Scene *s);
static inline void   engine_quit(Engine *e) { e->running = false; }
static inline Scene *engine_top(Engine *e)  { return e->stack_len ? e->stack[e->stack_len - 1] : NULL; }

void engine_toggle_fullscreen(Engine *e);
void engine_text_input(Engine *e, bool on);   /* deliver typed characters into Input.text */
