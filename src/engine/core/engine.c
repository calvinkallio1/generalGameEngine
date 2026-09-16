#include "engine.h"
#include "assets.h"
#include "audio.h"
#include "text.h"
#include "debug.h"
#include <string.h>
#include <stdio.h>

static SDL_Gamepad *gamepad;
static bool         want_gamepad;

/* ---------------------------------------------------------------- init / shutdown */

bool engine_init(Engine *e, const EngineConfig *cfg) {
  memset(e, 0, sizeof *e);

  e->logical_w = cfg->logical_w > 0 ? cfg->logical_w : 640;
  e->logical_h = cfg->logical_h > 0 ? cfg->logical_h : 360;
  int scale    = cfg->window_scale > 0 ? cfg->window_scale : 2;
  int rate     = cfg->tick_rate > 0 ? cfg->tick_rate : 60;
  e->ns_per_tick = 1000000000ULL / (uint64_t)rate;
  e->dt          = 1.0f / (float)rate;
  e->user        = cfg->user;
  e->pause_scene = cfg->pause_scene;
  e->clear_color = cfg->clear_color.a ? cfg->clear_color : (SDL_Color){ 20, 20, 30, 255 };
  want_gamepad   = cfg->gamepad;

  Uint32 flags = SDL_INIT_VIDEO | (cfg->gamepad ? SDL_INIT_GAMEPAD : 0);
  if (!SDL_Init(flags)) { SDL_Log("SDL_Init: %s", SDL_GetError()); return false; }

  int v = SDL_GetVersion();
  SDL_Log("engine %d.%d.%d on SDL %d.%d.%d", ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH,
          SDL_VERSIONNUM_MAJOR(v), SDL_VERSIONNUM_MINOR(v), SDL_VERSIONNUM_MICRO(v));

  if (!SDL_CreateWindowAndRenderer(cfg->title ? cfg->title : "Game",
                                   e->logical_w * scale, e->logical_h * scale,
                                   SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY,
                                   &e->window, &e->renderer)) {
    SDL_Log("SDL_CreateWindowAndRenderer: %s", SDL_GetError());
    return false;
  }
  SDL_SetRenderVSync(e->renderer, 1);
  SDL_SetRenderLogicalPresentation(e->renderer, e->logical_w, e->logical_h, SDL_LOGICAL_PRESENTATION_LETTERBOX);
  SDL_GetWindowSize(e->window, &e->window_w, &e->window_h);
  e->focused = true;

  /* Services, in dependency order. Failures are logged and non-fatal. */
  assets_init(e->renderer);
  save_init(&e->save, cfg->org, cfg->app);
  if (cfg->audio) e->has_audio = audio_init();
  if (cfg->text)  e->has_text  = text_init();
  return true;
}

void engine_shutdown(Engine *e) {
  while (e->stack_len > 0) {
    Scene *s = e->stack[--e->stack_len];
    if (s->on_exit) s->on_exit(e, s);
  }
  if (e->has_text)  text_shutdown();
  if (e->has_audio) audio_shutdown();
  assets_unload_all();
  if (gamepad) { SDL_CloseGamepad(gamepad); gamepad = NULL; }
  SDL_DestroyRenderer(e->renderer);
  SDL_DestroyWindow(e->window);
  SDL_Quit();
}

/* ---------------------------------------------------------------- window helpers */

void engine_toggle_fullscreen(Engine *e) {
  bool fs = (SDL_GetWindowFlags(e->window) & SDL_WINDOW_FULLSCREEN) != 0;
  SDL_SetWindowFullscreen(e->window, !fs);
}

void engine_text_input(Engine *e, bool on) {
  if (on) SDL_StartTextInput(e->window); else SDL_StopTextInput(e->window);
}

/* ---------------------------------------------------------------- input */

static void poll_input(Engine *e) {
  Input *in = &e->input;

  memcpy(in->prev_keys, in->keys, sizeof in->keys);
  memcpy(in->prev_pad_buttons, in->pad_buttons, sizeof in->pad_buttons);
  in->prev_mouse = in->mouse;
  in->wheel = 0;
  in->text[0] = '\0';

  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    switch (ev.type) {
      case SDL_EVENT_QUIT:                in->quit_requested = true; break;
      case SDL_EVENT_MOUSE_WHEEL:         in->wheel += ev.wheel.y;   break;
      case SDL_EVENT_WINDOW_RESIZED:      e->window_w = ev.window.data1; e->window_h = ev.window.data2; break;
      case SDL_EVENT_WINDOW_FOCUS_GAINED: e->focused = true;  break;
      case SDL_EVENT_WINDOW_FOCUS_LOST:
        e->focused = false;
        if (e->pause_scene && engine_top(e) != e->pause_scene && e->pending_len == 0) engine_push(e, e->pause_scene);
        break;
      case SDL_EVENT_KEY_DOWN:
        if (ev.key.repeat) break;
        if (ev.key.scancode == SDL_SCANCODE_F11) engine_toggle_fullscreen(e);
        if (ev.key.scancode == SDL_SCANCODE_F3)  e->debug_overlay = !e->debug_overlay;
        break;
      case SDL_EVENT_TEXT_INPUT: {
        size_t have = strlen(in->text), add = strlen(ev.text.text);
        if (have + add < sizeof in->text) memcpy(in->text + have, ev.text.text, add + 1);
        break;
      }
      case SDL_EVENT_GAMEPAD_ADDED:
        if (want_gamepad && !gamepad) { gamepad = SDL_OpenGamepad(ev.gdevice.which); in->pad_connected = gamepad != NULL; }
        break;
      case SDL_EVENT_GAMEPAD_REMOVED:
        if (gamepad && SDL_GetGamepadID(gamepad) == ev.gdevice.which) {
          SDL_CloseGamepad(gamepad); gamepad = NULL; in->pad_connected = false;
          memset(in->pad_buttons, 0, sizeof in->pad_buttons); memset(in->pad_axis, 0, sizeof in->pad_axis);
        }
        break;
      default: break;
    }
  }

  int n = 0;
  const bool *held = SDL_GetKeyboardState(&n);
  memcpy(in->keys, held, (size_t)(n < SDL_SCANCODE_COUNT ? n : SDL_SCANCODE_COUNT));

  in->mouse = SDL_GetMouseState(&in->mouse_x, &in->mouse_y);
  SDL_RenderCoordinatesFromWindow(e->renderer, in->mouse_x, in->mouse_y, &in->mouse_x, &in->mouse_y);

  if (gamepad) {
    for (int b = 0; b < SDL_GAMEPAD_BUTTON_COUNT; ++b) in->pad_buttons[b] = SDL_GetGamepadButton(gamepad, (SDL_GamepadButton)b);
    for (int a = 0; a < SDL_GAMEPAD_AXIS_COUNT; ++a) {
      float v = SDL_GetGamepadAxis(gamepad, (SDL_GamepadAxis)a) / 32767.0f;
      in->pad_axis[a] = (v > 0.2f || v < -0.2f) ? v : 0.0f;   /* dead zone */
    }
  }
}

/* ---------------------------------------------------------------- scene stack */

void engine_push(Engine *e, Scene *s)    { if (e->pending_len < PENDING_MAX) e->pending[e->pending_len++] = (PendingOp){ OP_PUSH, s }; }
void engine_pop(Engine *e)               { if (e->pending_len < PENDING_MAX) e->pending[e->pending_len++] = (PendingOp){ OP_POP, NULL }; }
void engine_replace(Engine *e, Scene *s) { if (e->pending_len < PENDING_MAX) e->pending[e->pending_len++] = (PendingOp){ OP_REPLACE, s }; }

static void pop_now(Engine *e) {
  if (e->stack_len == 0) return;
  Scene *s = e->stack[--e->stack_len];
  if (s->on_exit) s->on_exit(e, s);
}

static void push_now(Engine *e, Scene *s) {
  if (!s || e->stack_len >= SCENE_STACK_MAX) return;
  e->stack[e->stack_len++] = s;
  if (s->on_enter) s->on_enter(e, s);
}

static void apply_pending(Engine *e) {
  for (int i = 0; i < e->pending_len; ++i) {
    PendingOp p = e->pending[i];
    switch (p.op) {
      case OP_POP:     pop_now(e);                       break;
      case OP_REPLACE: pop_now(e); push_now(e, p.scene); break;
      case OP_PUSH:    push_now(e, p.scene);             break;
    }
  }
  e->pending_len = 0;
}

/* ---------------------------------------------------------------- main loop */

void engine_run(Engine *e) {
  uint64_t prev = SDL_GetTicksNS();
  uint64_t acc  = 0;
  e->running = true;

  while (e->running) {
    if (!e->focused) SDL_Delay(16);   /* don't spin at 100% in the background */

    uint64_t now = SDL_GetTicksNS();
    uint64_t frame_ns = now - prev;
    acc += frame_ns;
    prev = now;
    if (acc > e->ns_per_tick * 8) acc = e->ns_per_tick * 8;
    if (frame_ns > 0) { float inst = 1e9f / (float)frame_ns; e->fps = e->fps == 0 ? inst : e->fps * 0.95f + inst * 0.05f; }

    poll_input(e);
    if (e->input.quit_requested) e->running = false;
    if (e->has_audio) audio_update();

    /* 1. Input: top scene only. */
    Scene *top = engine_top(e);
    if (top && top->handle_input) top->handle_input(e, top, &e->input);
    apply_pending(e);

    /* 2. Update: fixed timestep, top-down until a scene blocks. */
    while (acc >= e->ns_per_tick) {
      for (int i = e->stack_len - 1; i >= 0; --i) {
        Scene *s = e->stack[i];
        if (s->update) s->update(e, s, e->dt);
        if (s->blocks_update_below) break;
      }
      apply_pending(e);
      acc -= e->ns_per_tick;
    }

    /* 3. Render: from the lowest scene that blocks rendering, upward. */
    float alpha = (float)acc / (float)e->ns_per_tick;
    SDL_SetRenderDrawColor(e->renderer, e->clear_color.r, e->clear_color.g, e->clear_color.b, e->clear_color.a);
    SDL_RenderClear(e->renderer);
    int start = 0;
    for (int i = e->stack_len - 1; i >= 0; --i) if (e->stack[i]->blocks_render_below) { start = i; break; }
    for (int i = start; i < e->stack_len; ++i) {
      Scene *s = e->stack[i];
      if (s->render) s->render(e, s, alpha);
    }

    /* 4. Debug overlay (F3): queued shapes/text, then a status line. */
    if (e->debug_overlay) {
      char line[160];
      snprintf(line, sizeof line, "%.0f fps  scenes %d  top %s  pad %s", e->fps, e->stack_len,
               top ? top->name : "-", e->input.pad_connected ? "yes" : "no");
      debug_text(4, 4, "%s", line);
    }
    debug_flush(e->renderer, e->debug_overlay);

    SDL_RenderPresent(e->renderer);
    e->frame++;

    if (e->stack_len == 0) e->running = false;
  }
}
