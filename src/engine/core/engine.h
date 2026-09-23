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

/** Maximum number of scenes on the stack at once. A push beyond it is dropped and logged. Eight is plenty: gameplay, pause, options, confirm is four.
 *  @see engine_push, Engine
 */
#define SCENE_STACK_MAX 8
/** Maximum stack changes (push/pop/replace) that may be queued from one callback before they apply. Extra calls are dropped.
 *  @see engine_push, PendingOp
 */
#define PENDING_MAX     8

/** Everything the engine needs to know about your game, passed once to engine_init(). Zero fields take defaults, so a designated initializer naming only what you care about is the normal way to build one: (EngineConfig){ .title = "Cats", .user = &state, .audio = true }. The struct is read once during engine_init and never again; it may live on the stack of main().
 *  @field title window title; NULL -> "Game"
 *  @field logical_w width in logical pixels, the resolution your game code thinks in; 0 -> 640. The window is scaled to fit and letterboxed, so drawing never has to know the real window size.
 *  @field logical_h height in logical pixels; 0 -> 360
 *  @field window_scale initial window size = logical size x this; 0 -> 2. The window stays resizable and F11 toggles fullscreen.
 *  @field tick_rate fixed simulation updates per second; 0 -> 60. Scene.update receives dt = 1/tick_rate every call regardless of the display's refresh rate.
 *  @field user pointer to your game state (normally a static GameState in main.c). The engine stores it in Engine.user and never reads it; scenes cast it back with GameState *g = e->user.
 *  @field org organization name for the per-user save folder (SDL_GetPrefPath); with app, e.g. "mystudio". NULL -> saves are written to the current working directory.
 *  @field app application name for the per-user save folder; see org
 *  @field audio open the default audio device so audio_load/audio_play/audio_music work. If no device exists the engine logs it and Engine.has_audio stays false; audio calls then do nothing.
 *  @field text initialize SDL_ttf so text_load_font can open .ttf files. Only effective when the engine was built with ENGINE_WITH_TTF; otherwise fonts fall back to the built-in 8x8 font.
 *  @field gamepad open the first gamepad that appears (and any that replaces it). Input.pad_* fields stay zero without it.
 *  @field pause_scene a scene the engine pushes by itself when the window loses focus (alt-tab, clicking another window); NULL -> nothing happens. Normally your pause overlay.
 *  @field clear_color the color the frame is cleared to before any scene renders; an alpha of 0 (the zero value) -> dark blue-grey { 20, 20, 30 }
 *  @see engine_init, Engine, gge-engine
 */
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

/** The kind of queued scene-stack change. Games never build one directly; engine_push(), engine_pop() and engine_replace() do.
 *  @field OP_PUSH add a scene on top
 *  @field OP_POP remove the top scene
 *  @field OP_REPLACE pop the top scene, then push another
 *  @see PendingOp, engine_push
 */
typedef enum { OP_PUSH, OP_POP, OP_REPLACE } SceneOp;
/** One queued scene-stack change, stored in Engine.pending until the current callback returns. Internal; documented so the Engine struct reads clearly.
 *  @field op what to do
 *  @field scene the scene to push (NULL for a pop)
 *  @see SceneOp, Engine
 */
typedef struct PendingOp { SceneOp op; Scene *scene; } PendingOp;

/** The running engine: window, renderer, this frame's input, the scene stack, and the handful of values a scene reads every frame. One exists per game, created with engine_init() and passed as the first argument to every scene callback and most engine calls. Fields marked read-only are maintained by the engine; writing them does nothing useful or breaks the loop. Game code normally touches only input, user, save, logical_w/h, dt, fps, frame and debug_overlay.
 *  @field window the SDL window. Internal; the engine already handles resize, fullscreen (F11) and focus.
 *  @field renderer the SDL renderer. Internal; games draw through draw.h, sprite.h and text.h, which target it.
 *  @field input this frame's snapshot of every input device, also passed to handle_input. Read it from update via &e->input when movement must run at tick rate.
 *  @field running true while the main loop should continue. engine_quit() clears it; the loop also ends when the scene stack becomes empty.
 *  @field logical_w width of the logical canvas in pixels (from EngineConfig or the 640 default). Use it to center things and to size full-screen rectangles.
 *  @field logical_h height of the logical canvas in pixels
 *  @field window_w real window width in pixels, updated on resize. Rarely needed; drawing happens in logical pixels.
 *  @field window_h real window height in pixels
 *  @field ns_per_tick nanoseconds per fixed update (1e9 / tick_rate). Read-only.
 *  @field dt seconds per fixed update, the same value every Scene.update receives. Read-only.
 *  @field user your game state, exactly the pointer given as EngineConfig.user. Scenes cast it: GameState *g = e->user.
 *  @field save the save-slot directory handle for save_write()/save_load(); pass &e->save.
 *  @field has_audio true when the audio device opened; audio_* calls are silently ignored otherwise
 *  @field has_text true when SDL_ttf initialized; otherwise text_load_font returns the debug font
 *  @field focused true while the window has keyboard focus. The loop sleeps a little per frame when unfocused.
 *  @field debug_overlay whether the F3 overlay (fps, scene stack, queued debug_rect/debug_line/debug_text) is drawn; F3 toggles it, code may set it
 *  @field fps smoothed frames per second, for the overlay or your own HUD
 *  @field frame frames presented so far; a cheap "every N frames" counter
 *  @field clear_color color the frame is cleared to before rendering
 *  @field pause_scene the scene pushed on focus loss, or NULL
 *  @field stack the scene stack, bottom at index 0. Internal; use engine_top() and the push/pop/replace calls.
 *  @field stack_len number of scenes on the stack
 *  @field pending stack changes queued by engine_push/pop/replace, applied after the current callback
 *  @field pending_len number of queued changes
 *  @see engine_init, engine_run, EngineConfig, Scene, gge-engine
 */
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

/** Zero *e, start SDL, open the window and renderer at cfg->logical_w x logical_h scaled by window_scale, apply vsync and logical presentation, then start the services cfg asks for (assets, save, audio, text, gamepad). Zero fields in cfg take defaults (640x360, scale 2, 60 Hz). Call exactly once, before any other engine function; e is normally a static Engine in main() so it outlives every scene. Nothing is on the scene stack afterwards: push the first scene, then call engine_run().
 *  @return true on success; false if SDL or the window failed. A service failing (no audio device, TTF not built) is logged and leaves e->has_audio / e->has_text false.
 *  @see engine_run, engine_shutdown, EngineConfig, gge-engine
 */
bool engine_init(Engine *e, const EngineConfig *cfg);
/** Run the main loop: poll input, apply queued scene changes, run fixed-rate update ticks (up to 8 per frame to catch up), render with an interpolation alpha, present. Returns when engine_quit was called, the window was closed, or the scene stack is empty. Every frame: (1) input is polled into e->input, (2) the top scene's handle_input runs, (3) queued pushes/pops apply, (4) update(dt) runs zero or more times at the fixed rate, walking the stack top-down until a scene with blocks_update_below, applying queued changes after each tick, (5) render(alpha) runs bottom-up starting at the topmost scene with blocks_render_below, (6) the F3 debug overlay is flushed and the frame presented. F11 toggles fullscreen and F3 the overlay before any scene sees the key.
 *  @see engine_init, engine_quit, engine_push, gge-engine, gge-scenes
 */
void engine_run(Engine *e);
/** Pop every remaining scene (their on_exit runs, top first), stop services in reverse order (text, audio, textures, gamepad, renderer, window) and quit SDL. Safe to call after engine_run returns; call it exactly once. Sounds loaded with audio_load() that were not freed in an on_exit are leaked, harmlessly, at this point.
 *  @see engine_init, engine_run
 */
void engine_shutdown(Engine *e);

/** Queue s to be placed on top of the scene stack. Applied after the current callback returns, so it is safe from handle_input, update, or a menu action. The scene's on_enter runs when the push is applied, not when this is called. Scenes are singletons: pushing one already on the stack shares its state, and pushing it twice puts the same Scene struct on the stack twice, so guard with engine_top(e) != s when a repeat is possible. The scene below keeps rendering (and updating) unless s sets blocks_render_below (blocks_update_below).
 *  @return none; a full stack (8 scenes) or a full queue (8 changes in one callback) drops the push and logs.
 *  @see engine_pop, engine_replace, engine_top, Scene, gge-scenes
 */
void engine_push(Engine *e, Scene *s);
/** Queue removal of the top scene. Its on_exit runs when applied, after the current callback returns. A scene may pop itself (the usual way a pause menu or dialog closes). When the last scene is popped the loop ends, which is how a title screen's Quit item works when there is nothing beneath it. Popping with an empty stack is ignored.
 *  @see engine_push, engine_replace, engine_quit
 */
void engine_pop(Engine *e);
/** Queue a swap of the top scene for s: the old scene's on_exit runs, then s is pushed and its on_enter runs. Use for title -> game and game -> game-over transitions where the old scene should not remain underneath. Equivalent to engine_pop followed by engine_push in the same callback.
 *  @see engine_push, engine_pop
 */
void engine_replace(Engine *e, Scene *s);
/** End the main loop after the current frame. Usable from any scene or menu action. Scenes still on the stack get their on_exit from engine_shutdown().
 *  @see engine_run, engine_shutdown
 */
static inline void   engine_quit(Engine *e) { e->running = false; }
/** The scene currently on top of the stack, or NULL when empty. Useful to guard against pushing a scene twice: if (engine_top(e) != shop_scene()) engine_push(e, shop_scene()). Queued changes are not reflected until they apply.
 *  @return the top Scene pointer or NULL.
 *  @see engine_push, Scene
 */
static inline Scene *engine_top(Engine *e)  { return e->stack_len ? e->stack[e->stack_len - 1] : NULL; }

/** Switch between windowed and borderless fullscreen. Logical size is unchanged; the frame is letterboxed. Bound to F11 by the engine, so an options menu item usually just calls this.
 *  @see engine_init, EngineConfig
 */
void engine_toggle_fullscreen(Engine *e);
/** Turn delivery of typed characters on or off. While on, Input.text holds the characters typed this frame (UTF-8, at most 31 bytes; usually one character). Turn on in a scene's on_enter and off in on_exit. Letter keys still report as key presses while on, so a "type your name" scene should not also treat KEY_A as an action. Backspace and Enter are not delivered as text; read them with key_pressed().
 *  @see key_pressed, Input, gge-input
 */
void engine_text_input(Engine *e, bool on);   /* deliver typed characters into Input.text */
