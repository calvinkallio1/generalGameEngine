#pragma once
#include <stdbool.h>
#include "types.h"

#ifdef ENGINE_WITH_TTF
#include <SDL3_ttf/SDL_ttf.h>
/** A loaded TrueType font at one point size, returned by text_load_font(). A small value type: copy it freely, keep one static per size you use. A zero Font ((Font){0}, or the result of a failed load, or any Font when the engine was built without ENGINE_WITH_TTF) is valid everywhere a Font is accepted and means "the built-in 8x8 debug font". Fonts are never freed by the game; SDL_ttf shuts down with the engine.
 *  @field ttf the underlying font handle; NULL means the debug font
 *  @field line_h height of one line in pixels, as text_line_height() reports it
 *  @see text_load_font, text_draw, text_size, gge-text
 */
typedef struct Font { TTF_Font *ttf; float line_h; } Font;
#else
typedef struct Font { void *ttf; float line_h; } Font;   /* always NULL: debug font */
#endif

/** Load a TrueType font from assets/ (e.g. "fonts/pixel.ttf") at the given point size. Requires the engine built with ENGINE_WITH_TTF and EngineConfig.text = true. Load once per size in on_enter (or once for the whole game in the title scene's on_enter) and keep the Font in a static; loading the same file twice makes two independent fonts. Point size is in logical pixels, so 16 is 16 logical pixels tall regardless of window size.
 *  @return the Font, or a zero Font on failure (logged); a zero Font draws with the 8px debug font, ignoring the size, so the game stays playable with the file missing.
 *  @see text_draw, text_size, text_line_height, Font, gge-assets
 */
Font  text_load_font(const char *relative_path, float pt);   /* under assets/; zero Font on failure */
/** Draw s at x,y (top-left) in color with font f, on one line: no wrapping, no newline handling (draw each line yourself, text_line_height() apart). Rendered strings are cached per font/color/string (64 entries), so drawing the same label each frame is cheap; a label that changes every frame (a millisecond timer) re-renders every frame, which is fine for a few strings but not for hundreds. Empty strings draw nothing. Strings longer than 127 bytes never hit the cache and so re-render every frame; split long text into lines.
 *  @see text_size, text_line_height, draw_debug_text, Font
 */
void  text_draw(Font f, float x, float y, Color color, const char *s);
/** Measure s in font f, writing pixel width and height through w and h. Neither pointer may be NULL. Use to center (x = (e->logical_w - w) / 2), right-align, or lay out a box around text. For the debug font the answer is 8 x strlen by 8.
 *  @see text_draw, text_line_height
 */
void  text_size(Font f, const char *s, int *w, int *h);
/** Height of one line in font f, for stacking lines: y += text_line_height(f). 8 for the debug font.
 *  @return the line height in logical pixels.
 *  @see text_size, text_draw
 */
float text_line_height(Font f);
/** Draw with the built-in 8x8 font regardless of TTF availability. Identical to draw_debug_text(); provided so text code can stay in one module.
 *  @see draw_debug_text, text_draw
 */
void  text_draw_debug(float x, float y, Color color, const char *s);

/* engine-internal */
/** Engine-internal: initialize SDL_ttf when EngineConfig.text is set. Called by engine_init(); games never call it.
 *  @return true on success.
 *  @see engine_init
 */
bool text_init(void);
/** Engine-internal: free the string cache and quit SDL_ttf. Called by engine_shutdown(); games never call it.
 *  @see engine_shutdown
 */
void text_shutdown(void);
