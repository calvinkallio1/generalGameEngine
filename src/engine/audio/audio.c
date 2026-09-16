#include "audio.h"
#include "assets.h"
#include <stdio.h>

#define SFX_CHANNELS 8

static SDL_AudioDeviceID device;
static SDL_AudioStream  *sfx[SFX_CHANNELS];
static SDL_AudioStream  *music;
static const Sound      *music_src;
static float             master_sfx = 1.0f, master_music = 1.0f, music_gain = 1.0f;

bool audio_init(void) {
  if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) { SDL_Log("audio: %s", SDL_GetError()); return false; }
  device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
  if (!device) { SDL_Log("audio: %s", SDL_GetError()); return false; }
  return true;
}

void audio_shutdown(void) {
  for (int i = 0; i < SFX_CHANNELS; ++i) if (sfx[i]) { SDL_DestroyAudioStream(sfx[i]); sfx[i] = NULL; }
  if (music) { SDL_DestroyAudioStream(music); music = NULL; }
  if (device) { SDL_CloseAudioDevice(device); device = 0; }
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

Sound audio_load(const char *rel) {
  Sound s = {0};
  char full[640]; assets_path(rel, full, sizeof full);
  if (!SDL_LoadWAV(full, &s.spec, &s.data, &s.len)) SDL_Log("audio: %s: %s", full, SDL_GetError());
  return s;
}

void audio_free(Sound *s) { SDL_free(s->data); s->data = NULL; s->len = 0; }

static SDL_AudioStream *make_stream(const SDL_AudioSpec *spec, float gain) {
  SDL_AudioStream *st = SDL_CreateAudioStream(spec, NULL);   /* NULL dst: device format, converted for us */
  if (!st) return NULL;
  SDL_BindAudioStream(device, st);
  SDL_SetAudioStreamGain(st, gain);
  return st;
}

void audio_play(const Sound *s, float gain) {
  if (!device || !s || !s->data) return;
  for (int i = 0; i < SFX_CHANNELS; ++i) {
    if (sfx[i] && SDL_GetAudioStreamAvailable(sfx[i]) > 0) continue;   /* busy */
    if (sfx[i]) SDL_DestroyAudioStream(sfx[i]);
    sfx[i] = make_stream(&s->spec, gain * master_sfx);
    if (sfx[i]) SDL_PutAudioStreamData(sfx[i], s->data, (int)s->len);
    return;
  }
}

void audio_music(const Sound *s, float gain) {
  audio_music_stop();
  if (!device || !s || !s->data) return;
  music_src = s; music_gain = gain;
  music = make_stream(&s->spec, gain * master_music);
  if (music) SDL_PutAudioStreamData(music, s->data, (int)s->len);
}

void audio_music_stop(void) {
  if (music) { SDL_DestroyAudioStream(music); music = NULL; }
  music_src = NULL;
}

void audio_set_master(float sfx_gain, float mus_gain) {
  master_sfx = sfx_gain; master_music = mus_gain;
  if (music) SDL_SetAudioStreamGain(music, music_gain * master_music);
}

void audio_update(void) {
  if (music && music_src && SDL_GetAudioStreamAvailable(music) < (int)music_src->len)
    SDL_PutAudioStreamData(music, music_src->data, (int)music_src->len);
}
