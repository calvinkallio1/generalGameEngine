/* input.h */
#pragma once
#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct Input {
    bool keys[SDL_SCANCODE_COUNT];
    bool prev_keys[SDL_SCANCODE_COUNT];
    float mouse_x, mouse_y;
    float wheel;
    SDL_MouseButtonFlags mouse, prev_mouse;
    bool quit_requested;
} Input;

static inline bool key_down(const Input *in, SDL_Scancode s) {
  return in->keys[s]; 
}
static inline bool key_pressed(const Input *in, SDL_Scancode s) {
  return in->keys[s] && !in->prev_keys[s]; 
}
static inline bool key_released(const Input *in, SDL_Scancode s) {
  return !in->keys[s] && in->prev_keys[s]; 
}
static inline bool mouse_down(const Input *in, int b) { 
  return (in->mouse & SDL_BUTTON_MASK(b)) != 0; 
}
static inline bool mouse_pressed(const Input *in, int b) {
  return (in->mouse & SDL_BUTTON_MASK(b)) && !(in->prev_mouse & SDL_BUTTON_MASK(b)); 
}
