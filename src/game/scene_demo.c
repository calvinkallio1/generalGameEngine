#include "engine.h"
#include "scenes.h"

static float x;

static void handle_input(Engine *e, Scene *s, const Input *in) {
  (void)s;
  if (key_pressed(in, SDL_SCANCODE_ESCAPE)) {
    engine_quit(e);
  }
}

static void update(Engine *e, Scene *s, float dt) {
  (void)s;
  x += 100.0f * dt;
  if (x > (float)e->logical_w) {
    x = 0;
  }
}

static void render(Engine *e, Scene *s, float alpha) {
  (void)s;
  (void)alpha;
  
  SDL_FRect box = { x, (float)e->logical_h / 2, 16, 16 };
  SDL_SetRenderDrawColor(e->renderer, 240, 160, 60, 255);
  SDL_RenderFillRect(e->renderer, &box);
}

Scene *demo_scene(void) {
  static Scene s = {
    .name = "demo",
    .handle_input = handle_input,
    .update = update,
    .render = render,
    .blocks_update_below = true,
    .blocks_render_below = true,
  };
  return &s;
}
