#pragma once
#include <stdbool.h>

typedef struct Engine Engine;
typedef struct Input  Input;

/** A Scene is a unit of behavior: title screen, gameplay, pause menu, inventory, dialog box, cutscene. The engine keeps a stack of these and calls their callbacks every frame. Every callback is optional (NULL = skip). Scenes own behavior, not data: game state lives in Engine.user; a scene keeps only presentation state (menu highlight, camera, tweens, loaded sprites) in file-level statics of its own .c file, reset in on_enter. Each scene is a singleton: a static Scene inside a getter function such as title_scene() that returns its address, declared in scenes.h so any other scene can push it.
 *  @field name short label shown in the F3 overlay and useful in logs
 *  @field on_enter called once when the scene lands on the stack (after engine_push/engine_replace applies). Load assets, build menus, reset statics, snap the camera, start music, turn on text input.
 *  @field on_exit called once when the scene leaves the stack (pop, replace, or engine_shutdown). Free sounds, stop music, turn off text input. Not called for scenes underneath when another is pushed on top.
 *  @field handle_input called once per displayed frame, only for the top scene, with this frame's Input. React to one-shot presses here (open a menu, confirm, pause). Do not move things here: it runs at display rate, not tick rate.
 *  @field update called zero or more times per frame at the fixed tick rate with dt = 1/tick_rate seconds. Simulation lives here: movement, timers, collisions, AI. Runs for the top scene and each scene beneath it until one has blocks_update_below set.
 *  @field render called once per displayed frame with alpha in 0..1, how far the simulation is between the last tick and the next. Draw everything here in logical pixels; draw moving things at v2_lerp(prev, now, alpha) for smooth motion. alpha has nothing to do with transparency. Runs bottom-up from the topmost scene that has blocks_render_below set.
 *  @field blocks_update_below true: scenes beneath this one stop ticking while it is on top (pause menu, modal dialog, cutscene). false: they keep simulating (HUD, toast, minimap).
 *  @field blocks_render_below true: scenes beneath are not drawn at all (title, gameplay, game over: anything full-screen). false: they are drawn first and this scene paints over them (pause menu over the frozen game).
 *  @see engine_push, engine_pop, engine_replace, engine_top, Engine, gge-scenes
 */
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
