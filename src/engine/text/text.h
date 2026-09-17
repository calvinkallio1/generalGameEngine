#pragma once
#include <stdbool.h>
#include "types.h"

#ifdef ENGINE_WITH_TTF
#include <SDL3_ttf/SDL_ttf.h>
typedef struct Font { TTF_Font *ttf; float line_h; } Font;
#else
typedef struct Font { void *ttf; float line_h; } Font;   /* always NULL: debug font */
#endif

/** Load a TrueType font from assets/ at the given point size. Requires ENGINE_WITH_TTF and EngineConfig.text = true.
 *  @return the Font, or a zero Font on failure (logged); a zero Font draws with the 8px debug font, ignoring the size.
 *  @see text_draw, text_size, gge-assets
 */
Font  text_load_font(const char *relative_path, float pt);   /* under assets/; zero Font on failure */
/** Draw s at x,y (top-left) in color with font f. Rendered strings are cached per font/color/string, so drawing the same label each frame is cheap; a label that changes every frame is not.
 *  @see text_size, draw_debug_text
 */
void  text_draw(Font f, float x, float y, Color color, const char *s);
/** Measure s in font f, writing pixel width and height through w and h. Neither pointer may be NULL. Use to center or right-align text.
 *  @see text_draw, text_line_height
 */
void  text_size(Font f, const char *s, int *w, int *h);
/** Height of one line in font f, for stacking lines.
 *  @see text_size
 */
float text_line_height(Font f);
/** Draw with the built-in 8x8 font regardless of TTF availability.
 *  @see draw_debug_text
 */
void  text_draw_debug(float x, float y, Color color, const char *s);

/* engine-internal */
bool text_init(void);
void text_shutdown(void);
