#pragma once
#include "engine.h"
#include "mathx.h"
#include "text.h"

/** What one cutscene step does. Each kind reads different CutStep fields; the CUT_* macros fill them for you.
 *  @field CUT_WAIT pause for duration seconds
 *  @field CUT_MOVE ease *actor from where it is to `to` over duration seconds
 *  @field CUT_SAY show a dialog line (speaker, text) and wait for the player to dismiss it
 *  @field CUT_FADE_OUT fade the screen to black over duration seconds (the black stays until a fade in)
 *  @field CUT_FADE_IN fade from black back to the scene over duration seconds
 *  @field CUT_CALL call fn(e, ctx) once and move on immediately (change a level, give an item, play a sound)
 *  @see CutStep, cutscene_start
 */
typedef enum { CUT_WAIT, CUT_MOVE, CUT_SAY, CUT_FADE_OUT, CUT_FADE_IN, CUT_CALL } CutKind;

/** One instruction in a cutscene script. Write scripts with the CUT_* macros rather than filling fields by hand. A script with no actors can be a static const array: static const CutStep intro[] = { CUT_FADE_IN_S(1), CUT_SAY_LINE("Cat", "Finally, a warm spot."), CUT_DO(give_key), CUT_FADE_OUT_S(1) };. A script that moves an entity in GameState is filled in at run time, because the entity's address is not a compile-time constant: static CutStep walk[2]; walk[0] = (CutStep)CUT_MOVE_TO(&g->player.pos, 200, 100, 2); walk[1] = (CutStep)CUT_SAY_LINE("Cat", "Here."); (each macro expands to a brace initializer, so a cast turns it into a compound literal). Which fields matter depends on kind.
 *  @field kind what the step does
 *  @field duration seconds, for WAIT, MOVE and the fades
 *  @field actor pointer to the Vec2 to move, for MOVE; must stay valid for the cutscene's life (an entity in GameState is fine; a local is not)
 *  @field to destination for MOVE
 *  @field speaker name for SAY, or NULL
 *  @field text line for SAY
 *  @field fn callback for CALL; receives the Engine and the ctx passed to cutscene_start()
 *  @see CutKind, cutscene_start, CUT_WAIT_S
 */
typedef struct CutStep {
  CutKind     kind;
  float       duration;
  Vec2       *actor;
  Vec2        to;
  const char *speaker, *text;
  void      (*fn)(Engine *, void *ctx);
} CutStep;

/** CutStep initializer: pause the cutscene for sec seconds. Expands to a brace initializer, so it goes directly inside a CutStep array initializer, or becomes a value with a cast: (CutStep)CUT_WAIT_S(0.5f).
 *  @see CutStep, cutscene_start
 */
#define CUT_WAIT_S(sec)              { .kind = CUT_WAIT, .duration = (sec) }
/** CutStep initializer: ease the Vec2 that p points to from its current position to (x, y) over sec seconds with ease_in_out_quad. p is usually the pos of an entity in GameState, so this step must be built at run time (see CutStep).
 *  @see CutStep, cutscene_start, ease_in_out_quad
 */
#define CUT_MOVE_TO(p, x, y, sec)    { .kind = CUT_MOVE, .actor = (p), .to = { (x), (y) }, .duration = (sec) }
/** CutStep initializer: show a dialog line spoken by who (NULL for none) and wait until the player dismisses it.
 *  @see CutStep, dialog_show
 */
#define CUT_SAY_LINE(who, line)      { .kind = CUT_SAY, .speaker = (who), .text = (line) }
/** CutStep initializer: fade the screen to black over sec seconds. The screen stays black until a CUT_FADE_IN_S step or the cutscene ends.
 *  @see CUT_FADE_IN_S, CutStep
 */
#define CUT_FADE_OUT_S(sec)          { .kind = CUT_FADE_OUT, .duration = (sec) }
/** CutStep initializer: fade from black back to the scene over sec seconds. Start a cutscene with it after switching levels in the previous cutscene's on_finish for a clean transition.
 *  @see CUT_FADE_OUT_S, CutStep
 */
#define CUT_FADE_IN_S(sec)           { .kind = CUT_FADE_IN, .duration = (sec) }
/** CutStep initializer: call f(e, ctx) once, where ctx is the pointer given to cutscene_start(), then continue immediately. Use for anything the script needs to change in the game: give an item, set a flag, play a sound, teleport the player.
 *  @see CutStep, cutscene_start
 */
#define CUT_DO(f)                    { .kind = CUT_CALL, .fn = (f) }

/** Play a script: pushes the engine's built-in cutscene scene on top of the stack, which runs steps[0..count-1] in order and pops itself when the last one finishes, then calls on_finish(e, ctx) if given. The scene beneath keeps rendering (so the world stays visible under the fades and dialog) but stops updating, so nothing else moves while actors are eased around. ESCAPE or PAD_START skips the whole cutscene (on_finish still runs, but remaining CUT_CALL steps do not, so put anything the game needs in on_finish). font is used for dialog lines; a zero Font is the debug font. steps must stay valid until the cutscene ends, so use a static const array. Only one cutscene runs at a time; starting another replaces the script.
 *  @see cutscene_active, CutStep, CutKind, dialog_show, gge-cutscene
 */
void cutscene_start(Engine *e, const CutStep *steps, int count, void *ctx, Font font,
                    void (*on_finish)(Engine *, void *ctx));
/** True while a cutscene scene is on the stack. Use to suppress input handling or HUD elements in scenes beneath it.
 *  @return true while a cutscene is playing.
 *  @see cutscene_start
 */
bool cutscene_active(void);
