#pragma once
/* Engine-owned names for the handful of platform types a game touches. They are the
 * SDL types underneath (same memory layout, zero conversion), so the engine can hand
 * them to SDL directly, but game code never has to write "SDL_". */
#include <SDL3/SDL.h>
#include <stdint.h>

/** An RGBA color with 8-bit channels, 0..255 each. Alpha 255 is opaque, 0 is invisible; every draw call blends, so alpha between the two is translucent. Build one with rgb(), rgba() or a COLOR_* constant. A Color whose alpha is 0 is treated as "unset" by several modules (EngineConfig.clear_color, Menu.color), which then substitute a default.
 *  @field r red, 0..255
 *  @field g green, 0..255
 *  @field b blue, 0..255
 *  @field a alpha (opacity), 0..255
 *  @see rgb, rgba, draw_rect, gge-types
 */
typedef SDL_Color   Color;     /* { uint8_t r, g, b, a } */
/** An axis-aligned rectangle in float pixels: top-left corner plus size. Used for everything rectangular: draw destinations, texture source regions, hitboxes, camera views. y grows downward. Build one with rect(), rect_at() or rect_centered().
 *  @field x left edge
 *  @field y top edge
 *  @field w width
 *  @field h height
 *  @see rect, rect_at, rect_centered, rect_overlaps, gge-types
 */
typedef SDL_FRect   Rect;      /* { float x, y, w, h } */
/** A 2D point in float pixels. Rarely needed by games, which use Vec2 from mathx.h for positions; Point exists for the few SDL calls that want it.
 *  @field x horizontal
 *  @field y vertical
 *  @see Vec2, Rect
 */
typedef SDL_FPoint  Point;     /* { float x, y } */
/** An image living on the GPU, ready to draw. Opaque: games only ever hold a pointer obtained from assets_texture() and pass it to draw_texture(), sprite_from() or tilemap_load_csv(). Never free one; the asset cache owns every Texture and releases them all in engine_shutdown().
 *  @see assets_texture, draw_texture, sprite_from, gge-assets
 */
typedef SDL_Texture Texture;   /* opaque; from assets_texture */

/** How to mirror an image when drawing it. Combine with | for both axes (FLIP_H | FLIP_V). Flipping happens about the destination rectangle's center, after rotation.
 *  @field FLIP_NONE draw as stored
 *  @field FLIP_H mirror left-right (a right-facing sprite faces left)
 *  @field FLIP_V mirror top-bottom
 *  @see draw_texture_ex, sprite_draw
 */
typedef enum Flip {
  FLIP_NONE = SDL_FLIP_NONE,
  FLIP_H    = SDL_FLIP_HORIZONTAL,
  FLIP_V    = SDL_FLIP_VERTICAL,
} Flip;

/** Build an opaque Color from red, green and blue (0..255 each); alpha is 255.
 *  @return the Color.
 *  @see rgba, Color
 */
static inline Color rgb(uint8_t r, uint8_t g, uint8_t b)               { return (Color){ r, g, b, 255 }; }
/** Build a Color with an explicit alpha (0 invisible .. 255 opaque). rgba(0,0,0,160) over the whole screen is the usual "dim the game behind a pause menu".
 *  @return the Color.
 *  @see rgb, Color
 */
static inline Color rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)   { return (Color){ r, g, b, a }; }
/** Build a Rect from a top-left corner and a size. Equivalent to the compound literal (Rect){ x, y, w, h } but readable inside an expression.
 *  @return the Rect.
 *  @see rect_at, rect_centered, Rect
 */
static inline Rect  rect(float x, float y, float w, float h)            { return (Rect){ x, y, w, h }; }

#define COLOR_WHITE   ((Color){ 255, 255, 255, 255 })
#define COLOR_BLACK   ((Color){   0,   0,   0, 255 })
#define COLOR_RED     ((Color){ 255,  60,  60, 255 })
#define COLOR_GREEN   ((Color){  60, 220,  90, 255 })
#define COLOR_BLUE    ((Color){  70, 130, 255, 255 })
#define COLOR_YELLOW  ((Color){ 255, 220,  80, 255 })
#define COLOR_GREY    ((Color){ 160, 160, 160, 255 })
#define COLOR_CLEAR   ((Color){   0,   0,   0,   0 })
