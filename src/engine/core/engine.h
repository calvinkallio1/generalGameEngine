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

/** Zero *e, start SDL, open the window and renderer at cfg->logical_w x logical_h scaled by window_scale, apply vsync and logical presentation, then start the services cfg asks for (assets, save, audio, text, gamepad). Zero fields in cfg take defaults (640x360, scale 2, 60 Hz).
 *  @return true on success; false if SDL or the window failed. A service failing (no audio device, TTF not built) is logged and leaves e->has_audio / e->has_text false.
 *  @see engine_run, engine_shutdown, gge-scenes
 */
bool engine_init(Engine *e, const EngineConfig *cfg);
/** Run the main loop: poll input, apply queued scene changes, run fixed-rate update ticks (up to 8 per frame to catch up), render with an interpolation alpha, present. Returns when engine_quit was called or the scene stack is empty.
 *  @see engine_init, engine_quit, gge-scenes
 */
void engine_run(Engine *e);
/** Pop every remaining scene (their on_exit runs), stop services in reverse order, and quit SDL. Safe to call after engine_run returns.
 *  @see engine_init
 */
void engine_shutdown(Engine *e);

/** Queue s to be placed on top of the scene stack. Applied after the current callback returns, so it is safe from handle_input, update, or a menu action. The scene's on_enter runs when the push is applied, not when this is called. Scenes are singletons: pushing one already on the stack shares its state.
 *  @return none; a full stack (8 scenes) drops the push and logs.
 *  @see engine_pop, engine_replace, engine_top, gge-scenes
 */
void engine_push(Engine *e, Scene *s);
/** Queue removal of the top scene. Its on_exit runs when applied. A scene may pop itself. When the last scene is popped the loop ends.
 *  @see engine_push, engine_replace
 */
void engine_pop(Engine *e);
/** Queue a swap of the top scene for s: the old scene's on_exit runs, then s is pushed. Use for title -> game transitions where the old scene should not remain underneath.
 *  @see engine_push, engine_pop
 */
void engine_replace(Engine *e, Scene *s);
/** End the main loop after the current frame. Usable from any scene or menu action.
 *  @see engine_run
 */
static inline void   engine_quit(Engine *e) { e->running = false; }
/** The scene currently on top of the stack, or NULL when empty. Useful to guard against pushing a scene twice: if (engine_top(e) != shop_scene()) ...
 *  @see engine_push
 */
static inline Scene *engine_top(Engine *e)  { return e->stack_len ? e->stack[e->stack_len - 1] : NULL; }

/** Switch between windowed and borderless fullscreen. Logical size is unchanged; the frame is letterboxed. Bound to F11 by the engine.
 *  @see engine_init
 */
void engine_toggle_fullscreen(Engine *e);
/** Turn delivery of typed characters on or off. While on, Input.text holds the characters typed this frame (UTF-8, short). Turn on in a scene's on_enter and off in on_exit. Letter keys still report as key presses while on.
 *  @see key_pressed, gge-input
 */
void engine_text_input(Engine *e, bool on);   /* deliver typed characters into Input.text */
