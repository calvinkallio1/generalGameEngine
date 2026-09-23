#pragma once
#include "engine.h"
#include "text.h"

/** A speech box at the bottom of the screen that types its text out character by character and waits for the player to dismiss it. One Dialog shows one line; a conversation is a sequence of dialog_show() calls driven by dialog_done(), or a cutscene of CUT_SAY steps. Keep one in a scene static (it holds pointers). Set font and chars_per_sec once; dialog_show() fills the rest.
 *  @field speaker name drawn in yellow above the text, or NULL for none
 *  @field text the line; must stay valid while active (a string literal or static buffer)
 *  @field chars_shown how many characters have been revealed so far (fractional)
 *  @field chars_per_sec typing speed; 0 = 40
 *  @field active true from dialog_show() until the player dismisses the completed line
 *  @field font font for both speaker and text; a zero Font is the debug font
 *  @see dialog_show, dialog_update, dialog_render, dialog_done, gge-dialog
 */
typedef struct Dialog {
  const char *speaker;
  const char *text;
  float chars_shown;
  float chars_per_sec;    /* 0 = 40 */
  bool  active;
  Font  font;             /* zero = debug font */
} Dialog;

/** Start showing a line: sets speaker and text, rewinds the typewriter and marks the dialog active. speaker may be NULL. Both strings are kept by pointer, not copied, so pass literals, static buffers, or GameState arrays, never a local. Calling it while a line is active replaces that line immediately.
 *  @see dialog_update, dialog_done, Dialog
 */
void dialog_show(Dialog *d, const char *speaker, const char *text);
/** Advance the typewriter by dt and handle dismissal. SPACE, ENTER, a left click, or PAD_SOUTH pressed this frame first completes the line (if still typing) and then, on the next press, closes it. Call from the owning scene's handle_input with e->dt, or from update with &e->input, but from only one of them. Does nothing when inactive.
 *  @see dialog_show, dialog_done, key_pressed
 */
void dialog_update(Dialog *d, const Input *in, float dt);
/** Draw the box: a translucent black panel with a white outline across the bottom of the logical screen, the speaker in yellow, and up to three word-wrapped lines of the revealed text. Text beyond three lines is cut off; keep lines short or split them. Draws nothing when inactive. Call last in render so it sits above the world.
 *  @see dialog_update, text_draw
 */
void dialog_render(Dialog *d, Engine *e);
/** True when no line is showing: initially, and after the player dismissed the last one. Poll it in update to start the next line of a conversation or to resume gameplay.
 *  @return !d->active.
 *  @see dialog_show, dialog_update
 */
static inline bool dialog_done(const Dialog *d) { return !d->active; }
