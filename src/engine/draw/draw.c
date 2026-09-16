#include "draw.h"

static SDL_Renderer *renderer;

void          draw_init(SDL_Renderer *r) { renderer = r; SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND); }
SDL_Renderer *draw_renderer(void)        { return renderer; }

static inline void set(Color c) { SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a); }

void draw_clear(Color c)                          { set(c); SDL_RenderClear(renderer); }
void draw_rect(Rect r, Color c)                   { set(c); SDL_RenderFillRect(renderer, &r); }
void draw_rect_outline(Rect r, Color c)           { set(c); SDL_RenderRect(renderer, &r); }
void draw_line(float x1, float y1, float x2, float y2, Color c) { set(c); SDL_RenderLine(renderer, x1, y1, x2, y2); }
void draw_point(float x, float y, Color c)        { set(c); SDL_RenderPoint(renderer, x, y); }
void draw_texture(Texture *t, const Rect *src, Rect dst) { if (t) SDL_RenderTexture(renderer, t, src, &dst); }
void draw_texture_ex(Texture *t, const Rect *src, Rect dst, double angle, Flip flip) {
  if (t) SDL_RenderTextureRotated(renderer, t, src, &dst, angle, NULL, (SDL_FlipMode)flip);
}
void draw_debug_text(float x, float y, Color c, const char *s) { set(c); SDL_RenderDebugText(renderer, x, y, s); }
