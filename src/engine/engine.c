#include "engine.h"
#include <string.h>

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

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL_Init: %s", SDL_GetError());
    return false;
  }
  if (!SDL_CreateWindowAndRenderer(cfg->title ? cfg->title : "Game",
                                   e->logical_w * scale, e->logical_h * scale,
                                   SDL_WINDOW_RESIZABLE, &e->window, &e->renderer)) {
    SDL_Log("SDL_CreateWindowAndRenderer: %s", SDL_GetError());
    return false;
  }
  /* Wait for refresh before presenting; stops tearing and caps the loop at the display rate. */
  SDL_SetRenderVSync(e->renderer, 1);
  /* Game code draws in logical_w x logical_h; SDL scales and letterboxes to the real window. */
  SDL_SetRenderLogicalPresentation(e->renderer, e->logical_w, e->logical_h,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);
  return true;
}

void engine_shutdown(Engine *e) {
  /* Give every scene still on the stack a chance to clean up. */
  while (e->stack_len > 0) {
    Scene *s = e->stack[--e->stack_len];
    if (s->on_exit) s->on_exit(e, s);
  }
  SDL_DestroyRenderer(e->renderer);
  SDL_DestroyWindow(e->window);
  SDL_Quit();
}

/* ---------------------------------------------------------------- input */

static void poll_input(Engine *e) {
  Input *in = &e->input;

  memcpy(in->prev_keys, in->keys, sizeof in->keys);
  in->prev_mouse = in->mouse;
  in->wheel = 0;

  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    switch (ev.type) {
      case SDL_EVENT_QUIT:        in->quit_requested = true; break;
      case SDL_EVENT_MOUSE_WHEEL: in->wheel += ev.wheel.y;   break;
      default: break;
    }
  }

  int n = 0;
  const bool *held = SDL_GetKeyboardState(&n);
  memcpy(in->keys, held, (size_t)n);

  in->mouse = SDL_GetMouseState(&in->mouse_x, &in->mouse_y);
  SDL_RenderCoordinatesFromWindow(e->renderer, in->mouse_x, in->mouse_y, &in->mouse_x, &in->mouse_y);
}

/* ---------------------------------------------------------------- scene stack */

void engine_push(Engine *e, Scene *s) {
  if (e->pending_len < PENDING_MAX)
    e->pending[e->pending_len++] = (PendingOp){ OP_PUSH, s };
}

void engine_pop(Engine *e) {
  if (e->pending_len < PENDING_MAX)
    e->pending[e->pending_len++] = (PendingOp){ OP_POP, NULL };
}

void engine_replace(Engine *e, Scene *s) {
  if (e->pending_len < PENDING_MAX)
    e->pending[e->pending_len++] = (PendingOp){ OP_REPLACE, s };
}

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
    uint64_t now = SDL_GetTicksNS();
    acc += now - prev;
    prev = now;
    if (acc > e->ns_per_tick * 8) acc = e->ns_per_tick * 8;   /* spiral-of-death guard */

    poll_input(e);
    if (e->input.quit_requested) e->running = false;

    /* 1. Input: top scene only. */
    if (e->stack_len > 0) {
      Scene *top = e->stack[e->stack_len - 1];
      if (top->handle_input) top->handle_input(e, top, &e->input);
    }
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
    SDL_SetRenderDrawColor(e->renderer, 20, 20, 30, 255);
    SDL_RenderClear(e->renderer);

    int start = 0;
    for (int i = e->stack_len - 1; i >= 0; --i) {
      if (e->stack[i]->blocks_render_below) { start = i; break; }
    }
    for (int i = start; i < e->stack_len; ++i) {
      Scene *s = e->stack[i];
      if (s->render) s->render(e, s, alpha);
    }
    SDL_RenderPresent(e->renderer);

    /* 4. Empty stack means nothing left to show. */
    if (e->stack_len == 0) e->running = false;
  }
}
