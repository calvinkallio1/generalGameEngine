#include "debug.h"
#include <stdarg.h>
#include <stdio.h>

#define MAX_SHAPES 256
#define MAX_TEXTS  32

typedef struct { bool is_line; SDL_FRect r; float x2, y2; SDL_Color c; } Shape;
typedef struct { float x, y; char s[128]; } Text;

static Shape shapes[MAX_SHAPES]; static int nshapes;
static Text  texts[MAX_TEXTS];   static int ntexts;

void debug_rect(SDL_FRect r, SDL_Color c) {
  if (nshapes < MAX_SHAPES) shapes[nshapes++] = (Shape){ .is_line = false, .r = r, .c = c };
}
void debug_line(float x1, float y1, float x2, float y2, SDL_Color c) {
  if (nshapes < MAX_SHAPES) shapes[nshapes++] = (Shape){ .is_line = true, .r = { x1, y1, 0, 0 }, .x2 = x2, .y2 = y2, .c = c };
}
void debug_text(float x, float y, const char *fmt, ...) {
  if (ntexts >= MAX_TEXTS) return;
  Text *t = &texts[ntexts++]; t->x = x; t->y = y;
  va_list ap; va_start(ap, fmt); vsnprintf(t->s, sizeof t->s, fmt, ap); va_end(ap);
}

void debug_flush(SDL_Renderer *r, bool draw) {
  if (draw) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < nshapes; ++i) {
      Shape *s = &shapes[i];
      SDL_SetRenderDrawColor(r, s->c.r, s->c.g, s->c.b, s->c.a);
      if (s->is_line) SDL_RenderLine(r, s->r.x, s->r.y, s->x2, s->y2);
      else            SDL_RenderRect(r, &s->r);
    }
    SDL_SetRenderDrawColor(r, 255, 255, 0, 255);
    for (int i = 0; i < ntexts; ++i) SDL_RenderDebugText(r, texts[i].x, texts[i].y, texts[i].s);
  }
  nshapes = 0; ntexts = 0;
}
