#pragma once
#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef ENGINE_WITH_TTF
#include <SDL3_ttf/SDL_ttf.h>
typedef struct Font { TTF_Font *ttf; float line_h; } Font;
#else
typedef struct Font { void *ttf; float line_h; } Font;   /* always NULL: debug font */
#endif

Font  text_load_font(const char *relative_path, float pt);   /* under assets/; zero Font on failure */
void  text_draw(SDL_Renderer *r, Font f, float x, float y, SDL_Color color, const char *s);
void  text_size(Font f, const char *s, int *w, int *h);
float text_line_height(Font f);
void  text_draw_debug(SDL_Renderer *r, float x, float y, SDL_Color color, const char *s);

/* engine-internal */
bool text_init(void);
void text_shutdown(void);
