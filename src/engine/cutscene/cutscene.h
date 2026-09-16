#pragma once
#include "engine.h"
#include "mathx.h"
#include "text.h"

typedef enum { CUT_WAIT, CUT_MOVE, CUT_SAY, CUT_FADE_OUT, CUT_FADE_IN, CUT_CALL } CutKind;

typedef struct CutStep {
  CutKind     kind;
  float       duration;
  Vec2       *actor;
  Vec2        to;
  const char *speaker, *text;
  void      (*fn)(Engine *, void *ctx);
} CutStep;

#define CUT_WAIT_S(sec)              { .kind = CUT_WAIT, .duration = (sec) }
#define CUT_MOVE_TO(p, x, y, sec)    { .kind = CUT_MOVE, .actor = (p), .to = { (x), (y) }, .duration = (sec) }
#define CUT_SAY_LINE(who, line)      { .kind = CUT_SAY, .speaker = (who), .text = (line) }
#define CUT_FADE_OUT_S(sec)          { .kind = CUT_FADE_OUT, .duration = (sec) }
#define CUT_FADE_IN_S(sec)           { .kind = CUT_FADE_IN, .duration = (sec) }
#define CUT_DO(f)                    { .kind = CUT_CALL, .fn = (f) }

void cutscene_start(Engine *e, const CutStep *steps, int count, void *ctx, Font font,
                    void (*on_finish)(Engine *, void *ctx));
bool cutscene_active(void);
