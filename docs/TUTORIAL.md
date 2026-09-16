# Engine tutorial

This is two things: an explanation of the engine that already exists in `src/engine/`,
and a step-by-step guide to building a game on top of it. Part 1 reads the code that is
there. Part 2 adds what a real game needs (state, menus, pause, saves) in an order where
each step produces something you can run.

Anything starting with `SDL_` is a library call and is explained the first time it shows
up. Everything else is ours.

---

## Part 1: What the engine is

### 1.1 Layout

```
src/engine/
  engine.h    Engine struct, EngineConfig, the public API
  engine.c    init/shutdown, input polling, scene stack, main loop
  input.h     Input snapshot + key_pressed() / mouse_pressed() helpers
  scene.h     the Scene interface
src/game/
  (empty)     a game's main.c, scenes.h and scene_*.c go here; see Part 2
```

The rule that keeps this reusable: the engine owns SDL, time, and the scene stack.
Your game owns behavior (scenes) and data (a struct you hand the engine through
`EngineConfig.user`). The engine never includes a game header.

### 1.2 The build (`CMakeLists.txt`)

The engine is a static library target called `engine`. Its include directory and its
SDL dependency are declared `PUBLIC`, so any target that does
`target_link_libraries(x PRIVATE engine)` automatically gets `#include "engine.h"` and
`#include <SDL3/SDL.h>` working, plus the right frameworks on macOS. You never add
`-I` flags for SDL by hand.

SDL is built from the submodule with `add_subdirectory(vendored/SDL)`. The `set(SDL_...)`
lines before it are options SDL's own CMake reads: static library, no tests, no examples.
`EXCLUDE_FROM_ALL` means SDL's targets build only when something links them.

Sources are collected with `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)`, so a new `.c`
file under `src/engine/` or `src/game/` is picked up on the next `cmake --build` with no
reconfigure. The game executable is only created `if(GAME_SOURCES)`, so an empty
`src/game/` builds just the library, and the first `main.c` you add makes the
executable appear.

Everyday commands:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug   # configure once
cmake --build build -j                          # build
./build/game_engine                              # once src/game/ has a main.c
cmake --build build --target clean               # clean objects, keep configuration
```

`Debug` is `-g -O0`: symbols, no optimization, what you want while developing. The
setting is saved in `build/CMakeCache.txt`, so you only pass it the first time.
`build/compile_commands.json` is what clangd reads; if your editor shows SDL errors
after deleting `build/`, reconfigure.

### 1.3 Configuration and init (`engine.h`, `engine.c`)

```c
EngineConfig cfg = {
  .title        = "My Game",
  .logical_w    = 640,   /* the resolution your code thinks in */
  .logical_h    = 360,
  .window_scale = 2,     /* initial window is 1280x720 */
  .tick_rate    = 60,    /* fixed updates per second */
  .user         = &my_state,
};
engine_init(&e, &cfg);
```

`engine_init` zeroes the `Engine` struct (`memset`), stores the config, then:

- `SDL_Init(SDL_INIT_VIDEO)` starts SDL's window/input systems. SDL3 functions return
  `bool`; on failure `SDL_GetError()` says why and `SDL_Log` prints it.
- `SDL_CreateWindowAndRenderer(title, w, h, flags, &window, &renderer)` makes both at
  once. A renderer is the object you draw with; it belongs to one window. The
  addresses are out-parameters: the function writes the new handles into your struct.
- `SDL_SetRenderVSync(renderer, 1)` waits for the monitor's refresh before showing a
  frame, which caps the loop at the display rate and prevents tearing.
- `SDL_SetRenderLogicalPresentation(renderer, w, h, LETTERBOX)` is the important one.
  It tells SDL "my game is `logical_w` x `logical_h`". You draw in that space; SDL scales
  and letterboxes to whatever size the real window is. Resize the window, nothing in
  your code changes.

`engine_shutdown` pops any scenes still on the stack (so their `on_exit` runs), then
destroys the renderer and window and calls `SDL_Quit()`.

### 1.4 Input (`input.h`, `poll_input` in `engine.c`)

Game code never calls `SDL_PollEvent`. Once per frame the engine fills an `Input`
struct, and scenes read it:

```c
typedef struct Input {
  bool keys[SDL_SCANCODE_COUNT];        /* held this frame, indexed by SDL_Scancode */
  bool prev_keys[SDL_SCANCODE_COUNT];   /* held last frame */
  float mouse_x, mouse_y;               /* in logical coordinates */
  float wheel;
  SDL_MouseButtonFlags mouse, prev_mouse;
  bool quit_requested;
} Input;
```

Keeping last frame's copy is what makes edge detection trivial:
`key_pressed(in, SDL_SCANCODE_SPACE)` is true on exactly one frame per tap,
`key_down` is true while held, `key_released` on the frame it comes up. Same for
`mouse_pressed(in, SDL_BUTTON_LEFT)`.

An `SDL_Scancode` is a physical key position (`SDL_SCANCODE_W` is the key where W is on
a US layout). Use scancodes for movement; they don't move around with keyboard layouts.

How `poll_input` fills it: `SDL_PollEvent(&ev)` pulls one queued event at a time until
the queue is empty. Only inherently one-shot things come from events (`SDL_EVENT_QUIT`
when the window is closed, `SDL_EVENT_MOUSE_WHEEL`). Keyboard state comes from
`SDL_GetKeyboardState(&n)`, a table of every key SDL is tracking, copied wholesale.
Mouse state comes from `SDL_GetMouseState(&x, &y)`, which returns a button bitmask and
writes the position, then `SDL_RenderCoordinatesFromWindow` converts window pixels into
logical coordinates so hit-testing matches what you drew.

### 1.5 Scenes (`scene.h`)

A scene is "what the game is doing right now": title screen, gameplay, pause menu,
settings. It's a struct of function pointers, all optional:

```c
typedef struct Scene {
  const char *name;
  void (*on_enter)(Engine *, struct Scene *);
  void (*on_exit)(Engine *, struct Scene *);
  void (*handle_input)(Engine *, struct Scene *, const Input *);
  void (*update)(Engine *, struct Scene *, float dt);
  void (*render)(Engine *, struct Scene *, float alpha);
  bool blocks_update_below;
  bool blocks_render_below;
} Scene;
```

Every scene file follows one shape: private state and private functions, all `static`,
and one public getter that returns a pointer to a `static Scene` (there is a complete
one in 2.0 below). A `static` local lives for the whole program, so returning its
address is safe and there's no malloc. The consequence is that each scene type is a
singleton, which is what you want for a title screen, a pause menu, a gameplay scene.

`.update = update` in the initializer stores the function's address (a function name
without parentheses is its address). Fields you don't list are zero, i.e. `NULL`, and the
engine checks for `NULL` before every call.

### 1.6 The scene stack and `PendingOp`

The engine keeps `Scene *stack[8]` plus a count. A stack rather than a single "current
scene" is what makes a pause menu trivial: push it on top of the game, and the game
freezes underneath but stays visible. Pop it and you're back mid-frame.

`engine_push`, `engine_pop`, `engine_replace` do not touch the stack. They append a
`PendingOp` (`{op, scene}`) to a small queue, and `apply_pending` performs the queued
changes later, at a point where the engine is not in the middle of calling into a
scene. That's the difference between a pause menu whose Resume button works and one
that pops itself while the engine is still iterating the stack. Queued ops run in
order, so "Quit to title" is just `engine_pop(e); engine_replace(e, title_scene());`.

### 1.7 The loop (`engine_run`)

Fixed timestep with an accumulator. `SDL_GetTicksNS()` is nanoseconds since init.

```
loop:
  acc += time since last frame          (capped at 8 ticks so a hitch can't snowball)
  poll_input
  handle_input on the TOP scene only    -> apply_pending
  while acc >= ns_per_tick:
      update every scene from the top down, stopping at the first with blocks_update_below
      apply_pending
      acc -= ns_per_tick
  alpha = acc / ns_per_tick             (0..1, how far into the next tick we are)
  clear
  render from the lowest scene with blocks_render_below upward, so top scenes overlay
  present
  if the stack is empty, stop
```

Update runs a whole number of times per frame at exactly `tick_rate` Hz, no matter how
fast the display is. Gameplay math uses `e->dt` (seconds per tick) and is
deterministic. Render happens once per frame and gets `alpha` so you can draw objects
slightly ahead of their last tick position for smoothness (see 2.7).

The three passes treat the stack differently on purpose: input to one scene, update
downward until blocked, render upward from the lowest visible. A pause menu has
`blocks_update_below = true, blocks_render_below = false`: it eats the input, freezes
the game, and draws over it.

Drawing calls you'll see everywhere: `SDL_SetRenderDrawColor(r, R, G, B, A)` sets the
color for subsequent calls; `SDL_RenderClear(r)` fills the frame with it;
`SDL_RenderFillRect(r, &rect)` fills an `SDL_FRect {x, y, w, h}`;
`SDL_RenderPresent(r)` shows the finished frame.

### 1.8 Verifying the engine

Put the three files from 2.0 into `src/game/`, build, run: a box crosses the screen in
about six seconds. Add `SDL_Delay(50);` before `SDL_RenderPresent` in `engine_run`,
rebuild: it stutters but still takes six seconds. That's the fixed timestep working.
Remove the delay.

Push `demo_scene()` twice from `main.c` and set `.blocks_update_below = false` in the
scene: the box moves at double speed because `update` runs for both stack entries. Set it
back to `true` and it's normal. That's the stack and the blocker logic working.

---

## Part 2: Building a game on it

Game code lives in `src/game/`, either in a clone of this repo used as a template or
in a game repo that pulls the engine in as a submodule (README). Names below are
placeholders: `Player`, `pickup`, and so on. Every file path in this part is relative
to `src/game/` unless it says `src/engine/`.

### 2.0 The smallest possible game

Three files. `scenes.h` declares every scene the game has; each scene is one
`scene_*.c`; `main.c` configures the engine and pushes the first scene.

```c
/* scenes.h */
#pragma once
#include "scene.h"
Scene *demo_scene(void);
```

```c
/* scene_demo.c */
#include "engine.h"
#include "scenes.h"

static float x;   /* scene-private; fine for a demo, wrong for real state (see 2.1) */

static void handle_input(Engine *e, Scene *s, const Input *in) {
  (void)s;
  if (key_pressed(in, SDL_SCANCODE_ESCAPE)) engine_quit(e);
}
static void update(Engine *e, Scene *s, float dt) {
  (void)s;
  x += 100.0f * dt;                        /* 100 logical px per second */
  if (x > (float)e->logical_w) x = 0;
}
static void render(Engine *e, Scene *s, float alpha) {
  (void)s; (void)alpha;
  SDL_FRect box = { x, (float)e->logical_h / 2, 16, 16 };
  SDL_SetRenderDrawColor(e->renderer, 240, 160, 60, 255);
  SDL_RenderFillRect(e->renderer, &box);
}

Scene *demo_scene(void) {
  static Scene s = {
    .name = "demo",
    .handle_input = handle_input, .update = update, .render = render,
    .blocks_update_below = true, .blocks_render_below = true,
  };
  return &s;
}
```

```c
/* main.c */
#include "engine.h"
#include "scenes.h"

int main(void) {
  static Engine e;   /* static: lives in the data segment, not the stack */
  EngineConfig cfg = {
    .title = "My Game", .logical_w = 640, .logical_h = 360,
    .window_scale = 2, .tick_rate = 60, .user = NULL,
  };
  if (!engine_init(&e, &cfg)) return 1;
  engine_push(&e, demo_scene());
  engine_run(&e);
  engine_shutdown(&e);
  return 0;
}
```

Build: the executable appears because `src/game/` now has sources. From here on,
each section adds or replaces files in this folder.

### 2.1 Game state

Every piece of mutable gameplay data goes in one plain struct: no pointers, no SDL
handles, fixed-capacity arrays with counts. This one decision makes save states,
undo, and replays cheap later.

```c
/* game_state.h */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#define MAX_PICKUPS 64

typedef struct { float x, y; } Vec2;
typedef struct { Vec2 pos, vel; int hp; bool facing_left; } Player;

typedef struct GameState {
  uint32_t tick, level;
  Player   player;
  int      pickup_count;
  Vec2     pickups[MAX_PICKUPS];
  bool     pickup_taken[MAX_PICKUPS];
  uint64_t rng;   /* own your RNG so it is part of the state */
} GameState;

static inline void game_state_init(GameState *g) {
  *g = (GameState){0};
  g->level = 1; g->player.hp = 3; g->rng = 0x9E3779B97F4A7C15ULL;
  g->player.pos = (Vec2){ 100, 100 };
}
```

Hand it to the engine and read it back in scenes:

```c
/* main.c */
static GameState state;
EngineConfig cfg = { .title = "My Game", .logical_w = 640, .logical_h = 360, .user = &state };

/* any scene */
GameState *g = e->user;
```

Textures, sounds, the renderer: none of that is state. They're resources reloaded from
disk, and they live on the engine or in the scene, not in `GameState`.

Quick-save is now struct assignment. In the gameplay scene:

```c
static GameState snapshot;
static bool have_snapshot;

static void handle_input(Engine *e, Scene *s, const Input *in) {
  (void)s;
  GameState *g = e->user;
  if (key_pressed(in, SDL_SCANCODE_F5)) { snapshot = *g; have_snapshot = true; }
  if (key_pressed(in, SDL_SCANCODE_F9) && have_snapshot) *g = snapshot;
}
```

Test: F5, move, F9, everything jumps back. If something doesn't restore, it's living in
the scene instead of `GameState`. That test is worth running every time you add state.

### 2.2 The gameplay scene

Keep the actual game logic in free functions over `GameState *` so it's testable
without a window. The scene is plumbing.

```c
/* scene_game.c */
#include "engine.h"
#include "scenes.h"
#include "game_state.h"

static void step_player(GameState *g, const Input *in, float dt) {
  float dir = (float)key_down(in, SDL_SCANCODE_RIGHT) - (float)key_down(in, SDL_SCANCODE_LEFT);
  g->player.vel.x += dir * 800.0f * dt;
  if (g->player.vel.x >  200) g->player.vel.x =  200;
  if (g->player.vel.x < -200) g->player.vel.x = -200;
  g->player.pos.x += g->player.vel.x * dt;
  g->player.vel.x *= 0.9f;
  if (dir != 0) g->player.facing_left = dir < 0;
}

static void update(Engine *e, Scene *s, float dt) {
  (void)s;
  GameState *g = e->user;
  step_player(g, &e->input, dt);
  g->tick++;
}

static void render(Engine *e, Scene *s, float alpha) {
  (void)s; (void)alpha;
  GameState *g = e->user;
  SDL_FRect box = { g->player.pos.x, g->player.pos.y, 16, 16 };
  SDL_SetRenderDrawColor(e->renderer, 240, 160, 60, 255);
  SDL_RenderFillRect(e->renderer, &box);
}

Scene *game_scene(void) {
  static Scene s = { .name = "game", .handle_input = handle_input, .update = update,
                     .render = render, .blocks_update_below = true, .blocks_render_below = true };
  return &s;
}
```

One wrinkle: `update` reads `e->input` directly, so a key tapped for less than one
tick at a low framerate could be missed. If it matters for jumps, have `handle_input`
latch a `jump_queued` flag that `update` consumes.

### 2.3 A menu widget (engine-level)

Menus are generic, so this belongs in `src/engine/` as `menu.h`/`menu.c`. It's a list
of labels with callbacks, a highlighted index, keyboard navigation, and mouse
hover/click by rectangle.

Text: SDL3 ships an 8x8 debug font, `SDL_RenderDebugText(r, x, y, "text")`, drawn in the
current draw color. It's enough until you want SDL_ttf, and swapping later doesn't
change the menu's interface.

```c
/* src/engine/menu.h */
#pragma once
#include "engine.h"
#define MENU_MAX_ITEMS 8

typedef struct { const char *label; void (*action)(Engine *); } MenuItem;
typedef struct Menu {
  MenuItem items[MENU_MAX_ITEMS];
  int   count, selected;
  float x, y, line_h;
} Menu;

void menu_add(Menu *m, const char *label, void (*action)(Engine *));
void menu_handle_input(Menu *m, Engine *e, const Input *in);
void menu_render(const Menu *m, SDL_Renderer *r);
```

```c
/* src/engine/menu.c */
#include "menu.h"

void menu_add(Menu *m, const char *label, void (*action)(Engine *)) {
  if (m->count < MENU_MAX_ITEMS) m->items[m->count++] = (MenuItem){ label, action };
}

static bool hit(const Menu *m, int i, float mx, float my) {
  SDL_FRect box = { m->x, m->y + i * m->line_h - 4, 200, m->line_h };
  SDL_FPoint p = { mx, my };
  return SDL_PointInRectFloat(&p, &box);
}

void menu_handle_input(Menu *m, Engine *e, const Input *in) {
  if (m->count == 0) return;
  if (key_pressed(in, SDL_SCANCODE_DOWN) || key_pressed(in, SDL_SCANCODE_S))
    m->selected = (m->selected + 1) % m->count;
  if (key_pressed(in, SDL_SCANCODE_UP) || key_pressed(in, SDL_SCANCODE_W))
    m->selected = (m->selected + m->count - 1) % m->count;
  for (int i = 0; i < m->count; ++i) {
    if (hit(m, i, in->mouse_x, in->mouse_y)) {
      m->selected = i;
      if (mouse_pressed(in, SDL_BUTTON_LEFT)) { m->items[i].action(e); return; }
    }
  }
  if (key_pressed(in, SDL_SCANCODE_RETURN) || key_pressed(in, SDL_SCANCODE_SPACE))
    m->items[m->selected].action(e);
}

void menu_render(const Menu *m, SDL_Renderer *r) {
  for (int i = 0; i < m->count; ++i) {
    bool sel = (i == m->selected);
    if (sel) SDL_SetRenderDrawColor(r, 255, 220, 80, 255);
    else     SDL_SetRenderDrawColor(r, 160, 160, 160, 255);
    SDL_RenderDebugText(r, m->x + (sel ? 12 : 0), m->y + i * m->line_h, m->items[i].label);
  }
}
```

Because the engine's CMake globs `src/engine/*.c`, dropping `menu.c` in is enough.

### 2.4 Title scene

A menu plus a heading. Actions are plain functions because C has no lambdas.

```c
/* scene_title.c */
#include "engine.h"
#include "menu.h"
#include "scenes.h"
#include "game_state.h"

static Menu menu;

static void act_new(Engine *e)  { game_state_init(e->user); engine_replace(e, game_scene()); }
static void act_quit(Engine *e) { engine_quit(e); }

static void on_enter(Engine *e, Scene *s) {
  (void)e; (void)s;
  menu = (Menu){ .x = 40, .y = 120, .line_h = 20 };
  menu_add(&menu, "New Game", act_new);
  menu_add(&menu, "Quit",     act_quit);
}
static void handle_input(Engine *e, Scene *s, const Input *in) { (void)s; menu_handle_input(&menu, e, in); }
static void render(Engine *e, Scene *s, float alpha) {
  (void)s; (void)alpha;
  SDL_SetRenderDrawColor(e->renderer, 255, 255, 255, 255);
  SDL_RenderDebugText(e->renderer, 40, 60, "MY GAME");
  menu_render(&menu, e->renderer);
}

Scene *title_scene(void) {
  static Scene s = { .name = "title", .on_enter = on_enter, .handle_input = handle_input,
                     .render = render, .blocks_update_below = true, .blocks_render_below = true };
  return &s;
}
```

`main.c` now pushes `title_scene()` instead of `demo_scene()`, and `scenes.h` declares
`title_scene`, `game_scene`, `pause_scene`. Nothing in the engine changes; that's the
point of the scene interface.

### 2.5 Pause scene

Copy the title scene, change the actions, flip one flag, dim the screen.

```c
/* scene_pause.c */
static void act_resume(Engine *e)   { engine_pop(e); }
static void act_to_title(Engine *e) { engine_pop(e); engine_replace(e, title_scene()); }

static void handle_input(Engine *e, Scene *s, const Input *in) {
  (void)s;
  if (key_pressed(in, SDL_SCANCODE_ESCAPE)) { engine_pop(e); return; }
  menu_handle_input(&menu, e, in);
}

static void render(Engine *e, Scene *s, float alpha) {
  (void)s; (void)alpha;
  SDL_SetRenderDrawBlendMode(e->renderer, SDL_BLENDMODE_BLEND);   /* honor alpha */
  SDL_SetRenderDrawColor(e->renderer, 0, 0, 0, 160);
  SDL_FRect full = { 0, 0, (float)e->logical_w, (float)e->logical_h };
  SDL_RenderFillRect(e->renderer, &full);
  menu_render(&menu, e->renderer);
}

Scene *pause_scene(void) {
  static Scene s = { /* ...as title... */ .blocks_update_below = true, .blocks_render_below = false };
  return &s;
}
```

`blocks_render_below = false` is the single line that makes it an overlay. In
`scene_game.c`'s `handle_input`: `if (key_pressed(in, SDL_SCANCODE_ESCAPE)) engine_push(e, pause_scene());`

Test the whole cycle: play, pause (player freezes, visible, dimmed), resume, pause,
quit to title, new game (state reset). A crash here is almost always a scene mutating
the stack directly instead of going through the pending queue.

### 2.6 Saves to disk (engine-level)

`GameState` has no pointers, so it's just bytes. The engine can save any such struct
without knowing its type: it takes a pointer and a size. This goes in
`src/engine/save.h`/`save.c`.

```c
/* src/engine/save.h */
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Save { char dir[512]; } Save;

void save_init(Save *s, const char *org, const char *app);
bool save_write(const Save *s, const char *slot, const void *data, size_t size, uint32_t version);
bool save_load (const Save *s, const char *slot, void *out, size_t size, uint32_t version);
```

```c
/* src/engine/save.c */
#include "save.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#define SAVE_MAGIC 0x45564153u   /* "SAVE" */
typedef struct { uint32_t magic, version, size; } SaveHeader;

void save_init(Save *s, const char *org, const char *app) {
  /* OS-correct per-user save folder, created if missing.
     macOS: ~/Library/Application Support/<org>/<app>/   Returns malloc'd; free with SDL_free. */
  char *p = SDL_GetPrefPath(org, app);
  snprintf(s->dir, sizeof s->dir, "%s", p ? p : "./");
  SDL_free(p);
}

static void path_for(const Save *s, const char *slot, char *out, size_t n) {
  snprintf(out, n, "%s%s.sav", s->dir, slot);
}

bool save_write(const Save *s, const char *slot, const void *data, size_t size, uint32_t version) {
  char path[640]; path_for(s, slot, path, sizeof path);
  SaveHeader h = { SAVE_MAGIC, version, (uint32_t)size };
  /* Two-part write without a temp buffer: SDL_IOStream is SDL's file handle. */
  SDL_IOStream *f = SDL_IOFromFile(path, "wb");
  if (!f) return false;
  bool ok = SDL_WriteIO(f, &h, sizeof h) == sizeof h && SDL_WriteIO(f, data, size) == size;
  SDL_CloseIO(f);
  return ok;
}

bool save_load(const Save *s, const char *slot, void *out, size_t size, uint32_t version) {
  char path[640]; path_for(s, slot, path, sizeof path);
  size_t n = 0;
  void *blob = SDL_LoadFile(path, &n);          /* whole file; NULL if missing */
  if (!blob) return false;
  bool ok = false;
  if (n == sizeof(SaveHeader) + size) {
    SaveHeader h; memcpy(&h, blob, sizeof h);
    if (h.magic == SAVE_MAGIC && h.version == version && h.size == size) {
      memcpy(out, (unsigned char *)blob + sizeof h, size);   /* only touch the caller's struct once validated */
      ok = true;
    }
  }
  SDL_free(blob);
  return ok;
}
```

Game side: a `Save` in your state-owning file (or on the engine's `user` struct),
`save_init(&save, "yourname", "yourgame")` at startup, then

```c
#define SAVE_VERSION 1
save_write(&save, "slot1", g, sizeof *g, SAVE_VERSION);          /* F6 in the game scene */
if (save_load(&save, "slot1", g, sizeof *g, SAVE_VERSION)) ...   /* "Continue" on the title */
```

Two honest caveats. Raw struct bytes depend on your compiler's padding and endianness,
fine for one platform's own saves, not for shipping cross-platform. And any change to
the struct invalidates old saves, which is why the header carries a version and the
size: a stale file is rejected, never misread. If you later need forward-compatible
saves, replace the internals with field-by-field writing; nothing else changes.

Test the sad paths: no file (Continue does nothing), a corrupted header byte, a save
from before you added a field. All silently rejected.

### 2.7 Smooth rendering with `alpha`

Update runs at 60 Hz; the display might be 120 Hz. Without interpolation, movement
looks like it stutters every other frame. Keep the previous tick's position (this is
presentation data, so it lives in the scene, not in `GameState`) and blend:

```c
static Vec2 prev_pos;

static void update(Engine *e, Scene *s, float dt) {
  GameState *g = e->user;
  prev_pos = g->player.pos;
  step_player(g, &e->input, dt);
  g->tick++;
}

static void render(Engine *e, Scene *s, float alpha) {
  GameState *g = e->user;
  Vec2 now = g->player.pos;
  SDL_FRect box = { prev_pos.x + (now.x - prev_pos.x) * alpha,
                    prev_pos.y + (now.y - prev_pos.y) * alpha, 16, 16 };
  ...
}
```

### 2.8 Sprites instead of rectangles

The SDL vocabulary: `SDL_LoadBMP(path)` gives an `SDL_Surface` (pixels in RAM).
`SDL_CreateTextureFromSurface(renderer, surface)` uploads it to an `SDL_Texture`
(pixels on the GPU); then `SDL_DestroySurface(surface)`. Draw with
`SDL_RenderTexture(renderer, texture, &src, &dst)` where `src` picks a frame out of a
sprite sheet and `dst` is where on screen. PNG needs SDL_image, added the same way as
SDL (submodule, `add_subdirectory`, link `SDL3_image::SDL3_image`).

A texture cache belongs on the engine: a small array of `{ char path[128]; SDL_Texture *tex; }`
with a lookup-or-load function. Textures are never part of `GameState`.
`SDL_GetBasePath()` gives the directory the executable is in, for finding `assets/`
regardless of the working directory.

---

## Where things go (the checklist)

- Reusable and game-agnostic (menu, save, texture cache): `src/engine/` in this repo.
- A scene: one `scene_*.c` in `src/game/`, declared in `src/game/scenes.h`.
- Mutable gameplay data: `GameState`, reached via `e->user`. If F9 doesn't restore it,
  it's in the wrong place.
- Presentation-only data (interpolation, animation timers, menu highlight): `static`
  in the scene.
- Never in the engine: a `switch` on which scene is active, or an include of a game header.
