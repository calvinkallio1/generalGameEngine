#pragma once
#include <SDL3/SDL.h>

#define LOG(...)   SDL_Log(__VA_ARGS__)
#define WARN(...)  SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define ASSERT(c)  SDL_assert(c)

void debug_rect(SDL_FRect r, SDL_Color c);
void debug_line(float x1, float y1, float x2, float y2, SDL_Color c);
void debug_text(float x, float y, const char *fmt, ...);

/* engine-internal: draws (if `draw`) and clears the queue every frame */
void debug_flush(SDL_Renderer *r, bool draw);
