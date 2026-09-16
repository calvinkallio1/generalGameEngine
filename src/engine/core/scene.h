#pragma once
#include <stdbool.h>

typedef struct Engine Engine;
typedef struct Input  Input;

/* A Scene is a unit of behavior: title screen, gameplay, pause menu, etc.
 * The engine keeps a stack of these. Every callback is optional (NULL = skip).
 * Scenes own behavior, not data: game state lives in Engine.user. */
typedef struct Scene {
  const char *name;

  void (*on_enter)(Engine *, struct Scene *);
  void (*on_exit)(Engine *, struct Scene *);
  void (*handle_input)(Engine *, struct Scene *, const Input *);
  void (*update)(Engine *, struct Scene *, float dt);
  void (*render)(Engine *, struct Scene *, float alpha);

  bool blocks_update_below;   /* true: scenes beneath this one stop ticking (pause menu) */
  bool blocks_render_below;   /* true: scenes beneath are not drawn (full-screen scene) */
} Scene;
