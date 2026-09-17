#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Save { char dir[512]; } Save;

/** Write size bytes of data to a slot file with a header carrying version. data must be pointer-free (a GameState).
 *  @return true on success; failures are logged.
 *  @see save_load, save_exists, gge-saves
 */
bool save_write(const Save *s, const char *slot, const void *data, size_t size, uint32_t version);
/** Read a slot into out. Fails (without touching out) if the file is missing, the version differs, or the size does not match.
 *  @return true on success.
 *  @see save_write
 */
bool save_load (const Save *s, const char *slot, void *out, size_t size, uint32_t version);
/** Whether a slot file exists; use to enable a Continue item.
 *  @see save_load
 */
bool save_exists(const Save *s, const char *slot);
/** Remove a slot file.
 *  @see save_write
 */
bool save_delete(const Save *s, const char *slot);

/* engine-internal */
void save_init(Save *s, const char *org, const char *app);
