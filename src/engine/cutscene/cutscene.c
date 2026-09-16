#include "cutscene.h"
#include "dialog.h"
#include "tween.h"
#include "draw.h"

static const CutStep *steps;
static int   count, index_;
static void *ctx;
static void (*finish_cb)(Engine *, void *);
static bool  active;

static float  elapsed;
static Vec2   move_from;
static Dialog dlg;
static float  fade;
static Tween  fade_tw;

static Scene *cutscene_scene(void);

static void begin_step(Engine *e) {
  elapsed = 0;
  if (index_ >= count) { engine_pop(e); return; }
  const CutStep *s = &steps[index_];
  switch (s->kind) {
    case CUT_MOVE:     if (s->actor) move_from = *s->actor; break;
    case CUT_SAY:      dialog_show(&dlg, s->speaker, s->text); break;
    case CUT_FADE_OUT: tween_start(&fade_tw, &fade, 1.0f, s->duration, ease_in_out_quad); break;
    case CUT_FADE_IN:  tween_start(&fade_tw, &fade, 0.0f, s->duration, ease_in_out_quad); break;
    case CUT_CALL:     if (s->fn) s->fn(e, ctx); break;
    default: break;
  }
}

static void next_step(Engine *e) { index_++; begin_step(e); }

void cutscene_start(Engine *e, const CutStep *st, int n, void *c, Font font, void (*on_finish)(Engine *, void *)) {
  steps = st; count = n; index_ = 0; ctx = c; finish_cb = on_finish;
  dlg = (Dialog){ .font = font, .chars_per_sec = 40 };
  fade = 0; fade_tw.active = false;
  engine_push(e, cutscene_scene());
}

bool cutscene_active(void) { return active; }

static void on_enter(Engine *e, Scene *s) { (void)s; active = true; begin_step(e); }

static void on_exit(Engine *e, Scene *s) {
  (void)s;
  active = false;
  if (finish_cb) { void (*cb)(Engine *, void *) = finish_cb; finish_cb = NULL; cb(e, ctx); }
}

static void handle_input(Engine *e, Scene *s, const Input *in) {
  (void)s;
  if (key_pressed(in, KEY_ESCAPE) || pad_pressed(in, PAD_START)) { engine_pop(e); return; }
  if (index_ < count && steps[index_].kind == CUT_SAY) dialog_update(&dlg, in, e->dt);
}

static void update(Engine *e, Scene *s, float dt) {
  (void)s;
  if (index_ >= count) return;
  const CutStep *st = &steps[index_];
  elapsed += dt;
  tween_update(&fade_tw, dt);
  switch (st->kind) {
    case CUT_WAIT:
      if (elapsed >= st->duration) next_step(e);
      break;
    case CUT_MOVE: {
      float t = st->duration > 0 ? clampf(elapsed / st->duration, 0, 1) : 1;
      if (st->actor) *st->actor = v2_lerp(move_from, st->to, ease_in_out_quad(t));
      if (t >= 1) next_step(e);
      break;
    }
    case CUT_SAY:
      if (dialog_done(&dlg)) next_step(e);
      break;
    case CUT_FADE_OUT: case CUT_FADE_IN:
      if (tween_done(&fade_tw)) next_step(e);
      break;
    case CUT_CALL:
      next_step(e);
      break;
  }
}

static void render(Engine *e, Scene *s, float alpha) {
  (void)s; (void)alpha;
  if (fade > 0) draw_rect(rect(0, 0, (float)e->logical_w, (float)e->logical_h), rgba(0, 0, 0, (uint8_t)(fade * 255)));
  dialog_render(&dlg, e);
}

static Scene *cutscene_scene(void) {
  static Scene sc = {
    .name = "cutscene",
    .on_enter = on_enter, .on_exit = on_exit, .handle_input = handle_input,
    .update = update, .render = render,
    .blocks_update_below = true, .blocks_render_below = false,
  };
  return &sc;
}
