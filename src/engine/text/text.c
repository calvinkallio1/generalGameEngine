#include "text.h"
#include "assets.h"
#include <stdio.h>
#include <string.h>

void text_draw_debug(SDL_Renderer *r, float x, float y, SDL_Color color, const char *s) {
  SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
  SDL_RenderDebugText(r, x, y, s);
}

#ifdef ENGINE_WITH_TTF

#define CACHE_SIZE 64
typedef struct { TTF_Font *font; char str[128]; SDL_Color color; SDL_Texture *tex; int w, h; unsigned last_used; } Cached;
static Cached   cache[CACHE_SIZE];
static unsigned clock_;
static bool     ready;

bool text_init(void) {
  if (!TTF_Init()) { SDL_Log("TTF_Init: %s", SDL_GetError()); return false; }
  ready = true;
  return true;
}

void text_shutdown(void) {
  for (int i = 0; i < CACHE_SIZE; ++i) if (cache[i].tex) { SDL_DestroyTexture(cache[i].tex); cache[i].tex = NULL; }
  if (ready) TTF_Quit();
  ready = false;
}

Font text_load_font(const char *rel, float pt) {
  Font f = {0};
  if (!ready) { SDL_Log("text: TTF not initialized (set EngineConfig.text = true)"); return f; }
  char full[640];
  f.ttf = TTF_OpenFont(assets_path(rel, full, sizeof full), pt);
  if (!f.ttf) { SDL_Log("font %s: %s", full, SDL_GetError()); return f; }
  f.line_h = (float)TTF_GetFontHeight(f.ttf);
  return f;
}

static bool same_color(SDL_Color a, SDL_Color b) { return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a; }

static Cached *lookup(SDL_Renderer *r, Font f, SDL_Color color, const char *s) {
  ++clock_;
  int victim = 0;
  for (int i = 0; i < CACHE_SIZE; ++i) {
    Cached *c = &cache[i];
    if (c->tex && c->font == f.ttf && same_color(c->color, color) && strcmp(c->str, s) == 0) { c->last_used = clock_; return c; }
    if (!c->tex) { victim = i; break; }
    if (c->last_used < cache[victim].last_used) victim = i;
  }
  Cached *c = &cache[victim];
  if (c->tex) { SDL_DestroyTexture(c->tex); c->tex = NULL; }
  SDL_Surface *surf = TTF_RenderText_Blended(f.ttf, s, 0, color);
  if (!surf) return NULL;
  c->tex = SDL_CreateTextureFromSurface(r, surf);
  c->w = surf->w; c->h = surf->h;
  SDL_DestroySurface(surf);
  c->font = f.ttf; c->color = color; c->last_used = clock_;
  snprintf(c->str, sizeof c->str, "%s", s);
  return c->tex ? c : NULL;
}

void text_draw(SDL_Renderer *r, Font f, float x, float y, SDL_Color color, const char *s) {
  if (!s || !*s) return;
  if (!f.ttf) { text_draw_debug(r, x, y, color, s); return; }
  Cached *c = lookup(r, f, color, s);
  if (!c) return;
  SDL_FRect dst = { x, y, (float)c->w, (float)c->h };
  SDL_RenderTexture(r, c->tex, NULL, &dst);
}

void text_size(Font f, const char *s, int *w, int *h) {
  if (f.ttf && TTF_GetStringSize(f.ttf, s, 0, w, h)) return;
  *w = (int)strlen(s) * 8; *h = 8;
}

float text_line_height(Font f) { return f.ttf ? f.line_h : 8.0f; }

#else  /* no SDL_ttf: everything is the debug font */

bool  text_init(void) { return true; }
void  text_shutdown(void) {}
Font  text_load_font(const char *rel, float pt) { (void)pt; SDL_Log("text: built without ENGINE_WITH_TTF; '%s' ignored", rel); return (Font){0}; }
void  text_draw(SDL_Renderer *r, Font f, float x, float y, SDL_Color color, const char *s) { (void)f; if (s && *s) text_draw_debug(r, x, y, color, s); }
void  text_size(Font f, const char *s, int *w, int *h) { (void)f; *w = (int)strlen(s) * 8; *h = 8; }
float text_line_height(Font f) { (void)f; return 8.0f; }

#endif
