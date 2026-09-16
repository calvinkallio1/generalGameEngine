#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Save { char dir[512]; } Save;

bool save_write(const Save *s, const char *slot, const void *data, size_t size, uint32_t version);
bool save_load (const Save *s, const char *slot, void *out, size_t size, uint32_t version);
bool save_exists(const Save *s, const char *slot);
bool save_delete(const Save *s, const char *slot);

/* engine-internal */
void save_init(Save *s, const char *org, const char *app);
