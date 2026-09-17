#pragma once
#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct Sound { Uint8 *data; Uint32 len; SDL_AudioSpec spec; } Sound;

/** Load a .wav from assets/. Requires EngineConfig.audio = true.
 *  @return a Sound; zero (and logged) on failure. Free with audio_free in on_exit.
 *  @see audio_play, audio_free, gge-assets
 */
Sound audio_load(const char *relative_path);      /* .wav under assets/ */
/** Release a Sound's samples. Do not free a sound that is currently the music.
 *  @see audio_load
 */
void  audio_free(Sound *s);
/** Play a sound once at gain 0..1 on a free channel. Dropped silently if every channel is busy.
 *  @see audio_music
 */
void  audio_play(const Sound *s, float gain);     /* fire-and-forget; dropped if all channels busy */
/** Loop s as the music track at gain, replacing any current music. s must outlive the playback.
 *  @see audio_music_stop
 */
void  audio_music(const Sound *s, float gain);    /* loops until replaced or stopped; `s` must outlive it */
/** Stop the music track.
 *  @see audio_music
 */
void  audio_music_stop(void);
/** Set master gains 0..1 for effects and music separately (an options menu writes these).
 *  @see audio_play
 */
void  audio_set_master(float sfx_gain, float music_gain);

/* engine-internal */
bool audio_init(void);
void audio_shutdown(void);
void audio_update(void);
