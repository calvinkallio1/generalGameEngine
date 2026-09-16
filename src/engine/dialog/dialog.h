#pragma once
#include "engine.h"
#include "text.h"

typedef struct Dialog {
  const char *speaker;
  const char *text;
  float chars_shown;
  float chars_per_sec;    /* 0 = 40 */
  bool  active;
  Font  font;             /* zero = debug font */
} Dialog;

void dialog_show(Dialog *d, const char *speaker, const char *text);
void dialog_update(Dialog *d, const Input *in, float dt);
void dialog_render(Dialog *d, Engine *e);
static inline bool dialog_done(const Dialog *d) { return !d->active; }
