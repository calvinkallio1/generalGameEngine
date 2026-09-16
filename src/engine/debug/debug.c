#include "debug.h"
#include "draw.h"
#include <stdarg.h>
#include <stdio.h>

#define MAX_SHAPES 256
#define MAX_TEXTS  32

typedef struct { bool is_line; Rect r; float x2, y2; Color c; } Shape;
typedef struct { float x, y; char s[128]; } Text;

static Shape shapes[MAX_SHAPES]; static int nshapes;
static Text  texts[MAX_TEXTS];   static int ntexts;

void debug_rect(Rect r, Color c) {
  if (nshapes < MAX_SHAPES) shapes[nshapes++] = (Shape){ .is_line = false, .r = r, .c = c };
}
void debug_line(float x1, float y1, float x2, float y2, Color c) {
  if (nshapes < MAX_SHAPES) shapes[nshapes++] = (Shape){ .is_line = true, .r = { x1, y1, 0, 0 }, .x2 = x2, .y2 = y2, .c = c };
}
void debug_text(float x, float y, const char *fmt, ...) {
  if (ntexts >= MAX_TEXTS) return;
  Text *t = &texts[ntexts++]; t->x = x; t->y = y;
  va_list ap; va_start(ap, fmt); vsnprintf(t->s, sizeof t->s, fmt, ap); va_end(ap);
}

void debug_flush(bool draw) {
  if (draw) {
    for (int i = 0; i < nshapes; ++i) {
      Shape *s = &shapes[i];
      if (s->is_line) draw_line(s->r.x, s->r.y, s->x2, s->y2, s->c);
      else            draw_rect_outline(s->r, s->c);
    }
    for (int i = 0; i < ntexts; ++i) draw_debug_text(texts[i].x, texts[i].y, COLOR_YELLOW, texts[i].s);
  }
  nshapes = 0; ntexts = 0;
}
