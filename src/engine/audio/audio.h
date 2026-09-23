#pragma once
#include <SDL3/SDL.h>
#include <stdbool.h>

/** A decoded .wav held in memory, ready to play any number of times. Returned by audio_load(); freed by audio_free(). Holds a pointer, so it belongs in a scene static (load in on_enter, free in on_exit), never in GameState. A zero Sound (failed load) is safe to pass everywhere and plays silence.
 *  @field data the samples
 *  @field len size of data in bytes
 *  @field spec sample format, channels and rate, as stored in the file
 *  @see audio_load, audio_play, audio_music, audio_free, gge-audio
 */
typedef struct Sound { Uint8 *data; Uint32 len; SDL_AudioSpec spec; } Sound;

/** Load a .wav from assets/ (e.g. "sfx/jump.wav"; any sample rate or channel count, converted at play time). Requires EngineConfig.audio = true; without it, or when no audio device exists, the result is a zero Sound and playing it is silent. Not cached: two loads of the same file are two copies, so load each sound once per scene, in on_enter, into a static.
 *  @return a Sound; zero (and logged) on failure. Free with audio_free in on_exit.
 *  @see audio_play, audio_music, audio_free, gge-assets
 */
Sound audio_load(const char *relative_path);      /* .wav under assets/ */
/** Release a Sound's samples and zero it. Do not free a sound that is currently the music (call audio_music_stop() first); effects already playing keep playing, since their samples were copied into the mixer.
 *  @see audio_load, audio_music_stop
 */
void  audio_free(Sound *s);
/** Play a sound once at gain 0..1 (1 = as recorded; the master effects gain is applied on top) on a free channel. Fire-and-forget: there is no handle, no stop, no way to tell when it ended. Eight effects can play at once; a ninth is dropped silently. Call from update (tick rate) or handle_input (on the press), never from render, which would play it every frame.
 *  @see audio_music, audio_set_master, Sound
 */
void  audio_play(const Sound *s, float gain);     /* fire-and-forget; dropped if all channels busy */
/** Loop s as the music track at gain, replacing any current music. s must outlive the playback: keep it in a static and stop the music before freeing it. Restarts from the beginning; there is no crossfade, so a level with the same track as the previous one should not call it again. Loops seamlessly if the file loops cleanly.
 *  @see audio_music_stop, audio_free, audio_set_master
 */
void  audio_music(const Sound *s, float gain);    /* loops until replaced or stopped; `s` must outlive it */
/** Stop the music track immediately. Safe when nothing is playing.
 *  @see audio_music
 */
void  audio_music_stop(void);
/** Set master gains 0..1 for effects and music separately (an options menu writes these; store the chosen values in GameState so a save remembers them). Music changes take effect at once; effects use the new gain from their next play.
 *  @see audio_play, audio_music
 */
void  audio_set_master(float sfx_gain, float music_gain);

/* engine-internal */
/** Engine-internal: open the default playback device. Called by engine_init() when EngineConfig.audio is set; games never call it.
 *  @return true on success.
 *  @see engine_init
 */
bool audio_init(void);
/** Engine-internal: destroy the mixer channels and close the device. Called by engine_shutdown(); games never call it.
 *  @see engine_shutdown
 */
void audio_shutdown(void);
/** Engine-internal: keep the music stream fed so it loops. Called once per frame by engine_run(); games never call it.
 *  @see engine_run
 */
void audio_update(void);
