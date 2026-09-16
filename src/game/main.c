/* src/game/main.c */
#include "engine.h"
#include "scenes.h"

int main(void) {
  static Engine e;
  EngineConfig cfg = {
    .title = "Engine", .logical_w = 640, .logical_h = 360,
    .window_scale = 2, .tick_rate = 60, .user = NULL,
  };
  if (!engine_init(&e, &cfg)) return 1;
  engine_push(&e, demo_scene());
  engine_run(&e);
  engine_shutdown(&e);
  return 0;
}
