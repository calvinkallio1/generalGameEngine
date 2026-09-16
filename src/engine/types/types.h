#pragma once
/* Engine-owned names for the handful of platform types a game touches. They are the
 * SDL types underneath (same memory layout, zero conversion), so the engine can hand
 * them to SDL directly, but game code never has to write "SDL_". */
#include <SDL3/SDL.h>
#include <stdint.h>

typedef SDL_Color   Color;     /* { uint8_t r, g, b, a } */
typedef SDL_FRect   Rect;      /* { float x, y, w, h } */
typedef SDL_FPoint  Point;     /* { float x, y } */
typedef SDL_Texture Texture;   /* opaque; from assets_texture */

typedef enum Flip {
  FLIP_NONE = SDL_FLIP_NONE,
  FLIP_H    = SDL_FLIP_HORIZONTAL,
  FLIP_V    = SDL_FLIP_VERTICAL,
} Flip;

static inline Color rgb(uint8_t r, uint8_t g, uint8_t b)               { return (Color){ r, g, b, 255 }; }
static inline Color rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)   { return (Color){ r, g, b, a }; }
static inline Rect  rect(float x, float y, float w, float h)            { return (Rect){ x, y, w, h }; }

#define COLOR_WHITE   ((Color){ 255, 255, 255, 255 })
#define COLOR_BLACK   ((Color){   0,   0,   0, 255 })
#define COLOR_RED     ((Color){ 255,  60,  60, 255 })
#define COLOR_GREEN   ((Color){  60, 220,  90, 255 })
#define COLOR_BLUE    ((Color){  70, 130, 255, 255 })
#define COLOR_YELLOW  ((Color){ 255, 220,  80, 255 })
#define COLOR_GREY    ((Color){ 160, 160, 160, 255 })
#define COLOR_CLEAR   ((Color){   0,   0,   0,   0 })
