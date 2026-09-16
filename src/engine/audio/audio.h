#pragma once
#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct Sound { Uint8 *data; Uint32 len; SDL_AudioSpec spec; } Sound;

Sound audio_load(const char *relative_path);      /* .wav under assets/ */
void  audio_free(Sound *s);
void  audio_play(const Sound *s, float gain);     /* fire-and-forget; dropped if all channels busy */
void  audio_music(const Sound *s, float gain);    /* loops until replaced or stopped; `s` must outlive it */
void  audio_music_stop(void);
void  audio_set_master(float sfx_gain, float music_gain);

/* engine-internal */
bool audio_init(void);
void audio_shutdown(void);
void audio_update(void);
